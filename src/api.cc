#include "api.h"

#include <string>
#include <vector>
#include <array>
#include <cstring>
#include <chrono>

#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <glpk.h>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <gzip/compress.hpp>

namespace lp_pp::rest
{

#define BAD_REQUEST(res, message)                                           \
    res.send(Http::Code::Bad_Request, createJsonError(message), MIME_JSON); \
    return

    using namespace Pistache;
    using namespace Pistache::Rest;

    using rapidjson::Document;
    using rapidjson::GetParseError_En;
    using rapidjson::StringBuffer;
    using rapidjson::Value;
    using rapidjson::Writer;
    using std::strcmp;
    using std::string;

    const auto MIME_JSON = Http::Mime::MediaType::fromString("application/json");

    const std::vector<string> origins{
        "http://localhost:8000",
        "https://lp-pp-vue.vercel.app",
        "https://lp-pp-vue-arturnikolaenko.vercel.app",
        "https://lp-pp-vue-git-main-arturnikolaenko.vercel.app"};

    const std::vector<string> solvers{
        "simplex",
        "glpk",
        "bnb",
        "homory"};

    SolverRouter::SolverRouter()
    {
        Routes::Get(*this, "/run-solver/:solver", Routes::bind(&SolverRouter::runSolver, this));

        Routes::Post(*this, "/set-model", Routes::bind(&SolverRouter::setModel, this));
        Routes::Post(*this, "/load-file/:fileName", Routes::bind(&SolverRouter::loadFile, this));
    }

    void SolverRouter::loadFile(const Rest::Request &req, Http::ResponseWriter res)
    {
        setHeaders(req, res);

        auto fileName = req.param(":fileName").as<string>();

        if (GLPKModel == nullptr)
        {
            GLPKModel = glp_create_prob();
        }

        string path = "data/" + fileName;

        if (glp_read_mps(GLPKModel, GLP_MPS_FILE, NULL, path.c_str()))
        {
            res.send(
                Http::Code::Not_Found,
                createJsonError(fmt::format("File '{}' not found.", fileName)),
                MIME_JSON);
            return;
        }

        filename = path;

        StringBuffer sb;
        Writer<StringBuffer> w(sb);

        size_t numRows = glp_get_num_rows(GLPKModel);
        size_t numCols = glp_get_num_cols(GLPKModel);

        w.StartObject();

        w.Key("name");
        w.String(glp_get_prob_name(GLPKModel));

        w.Key("type");
        int dir = glp_get_obj_dir(GLPKModel);

        if (dir > 0)
        {
            w.String("max");
        }
        else
        {
            w.String("min");
        }

        w.Key("variables");
        w.StartArray();
        for (size_t j = 1; j <= numCols; j++)
        {
            w.String(glp_get_col_name(GLPKModel, j));
        }
        w.EndArray();

        w.Key("objName");
        w.String(glp_get_obj_name(GLPKModel));

        w.Key("objective");
        w.StartArray();
        for (size_t j = 1; j <= numCols; j++)
        {
            w.Double(glp_get_obj_coef(GLPKModel, j));
        }
        w.EndArray();

        if (glp_get_num_int(GLPKModel) > 0)
        {
            w.Key("ints");
            w.StartArray();

            for (size_t j = 1; j <= glp_get_num_cols(GLPKModel); j++)
            {
                int kind = glp_get_col_kind(GLPKModel, j);

                if (kind == GLP_IV)
                {
                    w.Int(1);
                }
                else
                {
                    w.Int(0);
                }
            }
        }

        w.Key("consNames");
        w.StartArray();
        for (size_t i = 1; i <= numRows; i++)
        {
            w.String(glp_get_row_name(GLPKModel, i));
        }
        w.EndArray();

        w.Key("constraints");
        w.StartArray();

        auto *ind = new int[numRows + 1];
        auto *val = new double[numRows + 1];

        for (size_t i = 1; i <= numRows; i++)
        {
            int len = glp_get_mat_row(GLPKModel, i, ind, val);
            std::vector<double> v(numCols, {0.0});

            for (int j = 1; j <= len; j++)
            {
                v[ind[j] - 1] = val[j];
            }
            w.StartArray();
            for (const auto it : v)
            {
                w.Double(it);
            }
            w.EndArray();
        }

        delete[] ind;
        delete[] val;

        w.EndArray();

        w.Key("consTypes");

        w.StartArray();

        std::vector<double> bnds;

        for (int i = 1; i <= numRows; i++)
        {
            auto type = glp_get_row_type(GLPKModel, i);
            double bound = 0.0;
            string typeStr = "eq";

            switch (type)
            {
            case GLP_FR:
                typeStr = "fr";
                break;
            case GLP_FX:
                typeStr = "eq";
                bound = glp_get_row_lb(GLPKModel, i);
                break;
            case GLP_LO:
                typeStr = "gt";
                bound = glp_get_row_lb(GLPKModel, i);
                break;
            case GLP_UP:
                typeStr = "lt";
                bound = glp_get_row_ub(GLPKModel, i);
                break;
            }

            w.String(typeStr.c_str());
            bnds.push_back(bound);
        }

        w.EndArray();

        w.Key("consVals");
        w.StartArray();
        for (const auto b : bnds)
        {
            w.Double(b);
        }
        w.EndArray();

        w.EndObject();

        const char *str = sb.GetString();
        string comp = gzip::compress(str, std::strlen(str), Z_BEST_COMPRESSION);

        res.headers().add<Http::Header::ContentEncoding>(Http::Header::Encoding::Gzip);
        res.send(Http::Code::Created, comp, MIME3(Application, Json, Zip));
    }

    void SolverRouter::runSolver(const Rest::Request &req, Http::ResponseWriter res)
    {
        setHeaders(req, res);

        string solverName = req.param(":solver").as<string>();

        if (!std::count(solvers.begin(), solvers.end(), solverName))
        {
            res.send(Http::Code::Not_Found);
            return;
        }

        if (!ensureFile())
        {
            res.send(Http::Code::Not_Found, "Unable to open file: " + filename);
            return;
        }

        if (GLPKModel == nullptr)
        {
            res.send(Http::Code::Conflict, createJsonError("GLPK model is not set."), MIME_JSON);
            return;
        }

        const auto start = std::chrono::system_clock::now();
        const int code = glp_simplex(GLPKModel, NULL);
        const string fatal = processSimplexCode(code);

        if (!fatal.empty())
        {
            res.send(Http::Code::Internal_Server_Error, fatal);
            return;
        }

        const int status = glp_get_status(GLPKModel);
        std::string err = processSimplexStatus(status);

        StringBuffer b;
        Writer<StringBuffer> w(b);

        if (solverName == "simplex")
        {
            const auto end = std::chrono::system_clock::now();
            w.StartObject();

            size_t mem = 0;
            glp_mem_usage(NULL, NULL, &mem, NULL);

            w.Key("stats");
            createStats(w, start, end, mem);

            if (err.empty())
            {
                w.Key("objective");
                w.Double(glp_get_obj_val(GLPKModel));

                w.Key("variables");
                w.StartObject();

                for (size_t j = 1; j <= glp_get_num_cols(GLPKModel); j++)
                {
                    w.Key(glp_get_col_name(GLPKModel, j));
                    w.Double(glp_get_col_prim(GLPKModel, j));
                }

                w.EndObject();
            }

            w.EndObject();
        }
        else if (solverName == "glpk" || solverName == "bnb" || solverName == "homory")
        {
            glp_iocp param;
            glp_init_iocp(&param);

            if (solverName == "homory")
                param.gmi_cuts = GLP_ON;

            param.cb_func = [](glp_tree *t, void *stats)
            {
                IntStats *s = (IntStats *)stats;

                switch (glp_ios_reason(t))
                {
                case GLP_IROWGEN:
                    s->numCuts++;
                    break;
                case GLP_IBRANCH:
                    s->numBranches++;
                    break;
                }
            };

            IntStats s{0, 0};
            param.cb_info = &s;

            const int intCode = glp_intopt(GLPKModel, &param);
            const auto end = std::chrono::system_clock::now();

            const auto fatal = processMipCode(intCode);

            if (!fatal.empty())
            {
                res.send(Http::Code::Internal_Server_Error, fatal);
                return;
            }

            int intStatus = glp_mip_status(GLPKModel);
            err = processMipStatus(intStatus);

            w.StartObject();

            size_t mem = 0;
            glp_mem_usage(NULL, NULL, &mem, NULL);

            w.Key("stats");
            createStats(w, start, end, mem, &s);

            if (err.empty())
            {
                w.Key("objective");
                w.Double(glp_mip_obj_val(GLPKModel));

                w.Key("variables");
                w.StartObject();

                for (size_t j = 1; j <= glp_get_num_cols(GLPKModel); j++)
                {
                    w.Key(glp_get_col_name(GLPKModel, j));
                    w.Double(glp_mip_col_val(GLPKModel, j));
                }

                w.EndObject();
            }
            w.EndObject();

            s.numBranches = 0;
            s.numCuts = 0;
        }
        else if (solverName == "bnb")
        {

        }

        glp_delete_prob(GLPKModel);
        GLPKModel = nullptr;

        res.send(Http::Code::Ok, b.GetString(), MIME_JSON);
    }

    void SolverRouter::setModel(const Rest::Request &req, Http::ResponseWriter res)
    {
        setHeaders(req, res);

        Document model;
        model.Parse(req.body().c_str());

        if (model.HasParseError())
        {
            std::string err = fmt::format(
                "JSON parse error at [{}]: {} ",
                std::to_string(model.GetErrorOffset()),
                rapidjson::GetParseError_En(model.GetParseError()));

            BAD_REQUEST(res, err);
        }

        const string err = validateModel(model);

        if (!err.empty())
        {
            BAD_REQUEST(res, err);
        }

        filename = "";

        GLPKModel = glp_create_prob();

        jsonToGlpkModel(model, GLPKModel);

        res.send(Http::Code::Created);
    }

    void SolverRouter::setHeaders(const Rest::Request &req, Http::ResponseWriter &res)
    {
        res.headers().add<Http::Header::AccessControlAllowOrigin>("*");
        // res.headers().addRaw(Http::Header::Raw("Vary", "Origin"));
    }

    string SolverRouter::createJsonError(string message, string reason)
    {
        StringBuffer s;
        Writer<StringBuffer> w(s);

        w.StartObject();
        w.Key("error");
        w.String(message.c_str());

        if (!reason.empty())
        {
            w.Key("reason");
            w.String(reason.c_str());
        }

        w.EndObject();

        return s.GetString();
    }

    std::string SolverRouter::processSimplexStatus(int status)
    {
        if (status == GLP_OPT)
        {
            return "";
        }

        if (status == GLP_INFEAS || status == GLP_NOFEAS)
        {
            return createJsonError("Solution is infeasible", "infeasible");
        }

        if (status == GLP_UNBND)
        {
            return createJsonError("Solution is unbounded", "unbounded");
        }

        return createJsonError("Unable to obtain solution due to unknown reason.", "undefined");
    }

    std::string SolverRouter::processMipStatus(int status)
    {
        if (status != GLP_OPT)
        {
            return createJsonError("No integer feasible solution", "intunfeas");
        }

        return "";
    }

    bool SolverRouter::ensureFile()
    {
        if (!filename.empty())
        {
            if (GLPKModel == nullptr)
            {
                GLPKModel = glp_create_prob();
            }

            if (glp_read_mps(GLPKModel, GLP_MPS_FILE, NULL, filename.c_str()))
            {
                return false;
            }
        }

        return true;
    }

    void SolverRouter::jsonToGlpkModel(const Document &json, glp_prob *prob)
    {
        if (prob == nullptr)
        {
            prob = glp_create_prob();
        }

        const auto modelName = json["name"].GetString();
        const auto objName = json["objName"].GetString();

        glp_set_prob_name(prob, modelName);
        glp_set_obj_name(prob, objName);

        const auto opType = json["type"].GetString();

        if (!std::strcmp(opType, "max"))
        {
            glp_set_obj_dir(prob, GLP_MAX);
        }
        else
        {
            glp_set_obj_dir(prob, GLP_MIN);
        }

        const auto &vars = json["variables"].GetArray();
        const size_t numVars = vars.Size();
        const auto &obj = json["objective"].GetArray();

        glp_add_cols(prob, numVars);

        for (size_t j = 1; j <= numVars; j++)
        {
            glp_set_obj_coef(prob, j, obj[j - 1].GetDouble());
            glp_set_col_name(prob, j, vars[j - 1].GetString());
            glp_set_col_bnds(prob, j, GLP_LO, 0.0, 0.0);
        }

        if (json.HasMember("ints"))
        {
            const auto &ints = json["ints"].GetArray();

            for (size_t j = 1; j <= numVars; j++)
            {
                if (ints[j - 1].GetInt() > 0)
                {
                    glp_set_col_kind(GLPKModel, j, GLP_IV);
                }
            }
        }

        const auto &cNames = json["consNames"].GetArray();
        const size_t numCons = cNames.Size();

        const auto &cTypes = json["consTypes"].GetArray();
        const auto &consVals = json["consVals"].GetArray();
        const auto &consMat = json["constraints"].GetArray();

        glp_add_rows(prob, numCons);

        for (size_t i = 1; i <= numCons; i++)
        {
            glp_set_row_name(prob, i, cNames[i - 1].GetString());

            int boundType = GLP_FR;
            double lb = 0.0, ub = 0.0;

            if (!strcmp(cTypes[i - 1].GetString(), "lt"))
            {
                boundType = GLP_UP;
                ub = consVals[i - 1].GetDouble();
            }
            else if (!strcmp(cTypes[i - 1].GetString(), "gt"))
            {
                boundType = GLP_LO;
                lb = consVals[i - 1].GetDouble();
            }
            else if (!strcmp(cTypes[i - 1].GetString(), "eq"))
            {
                boundType = GLP_FX;
                ub = lb = consVals[i - 1].GetDouble();
            }

            glp_set_row_bnds(prob, i, boundType, lb, ub);
        }

        size_t total = numCons * numVars;

        auto *ia = new int[total + 1];
        auto *ja = new int[total + 1];
        auto *ra = new double[total + 1];

        size_t index = 1;

        for (size_t i = 1; i <= numCons; i++)
        {
            for (size_t j = 1; j <= numVars; j++)
            {
                ia[index] = i;
                ja[index] = j;
                ra[index] = consMat[i - 1].GetArray()[j - 1].GetDouble();

                index++;
            }
        }

        glp_load_matrix(prob, total - 1, ia, ja, ra);

        delete[] ia;
        delete[] ja;
        delete[] ra;
    }

    std::string SolverRouter::validateModel(const rapidjson::Document &model)
    {
        if (!ensureString(model, "name"))
        {
            return "Model name undefined or invalid (expected string)";
        }

        if (!ensureString(model, "type"))
        {
            return "Model optimization type undefined or invalid (expected string)";
        }

        string opType = model["type"].GetString();

        if (opType != "min" && opType != "max")
        {
            return "Model optimization type can be either 'min' or 'max'";
        }

        if (!ensureString(model, "objName"))
        {
            return "Objective name undefined or invalid (expected string)";
        }

        if (!ensureArray(model, "variables"))
        {
            return "Variable names undefined or invalid (expected array of string)";
        }

        const size_t numVars = model["variables"].GetArray().Size();

        for (const auto &it : model["variables"].GetArray())
        {
            if (!it.IsString())
            {
                return "Variable names must be array of string.";
            }
        }

        if (!ensureArray(model, "objective"))
        {
            return "Objective coefs undefined or invalid (expected array of double)";
        }

        const auto &obj = model["objective"].GetArray();

        if (obj.Size() != numVars)
        {
            return "Objective size does not correspond to amount of varibles.";
        }

        for (const auto &it : obj)
        {
            if (!it.IsNumber())
            {
                return "Objective coeficients must be array of double.";
            }
        }

        if (model.HasMember("ints"))
        {

            const auto &ints = model["ints"].GetArray();
            for (const auto &it : ints)
            {
                if (!it.IsInt())
                {
                    return "ints should be array of integer, where 0 stands for non int corresponding variable, and any value for int";
                }
            }
        }

        if (!ensureArray(model, "consNames"))
        {
            return "Constraint names undefined or invalid (expected array of string)";
        }

        const auto &cNames = model["consNames"].GetArray();
        size_t numCons = cNames.Size();

        for (const auto &it : cNames)
        {
            if (!it.IsString())
            {
                return "Constraint names must be array of string.";
            }
        }

        if (!ensureArray(model, "consTypes"))
        {
            return "Constraint types undefined or invalid (expected array of string).";
        }

        const auto &cTypes = model["consTypes"].GetArray();

        if (cTypes.Size() != numCons)
        {
            return "Constraint types size does not correspond to amount of constraints";
        }

        for (const auto &it : cTypes)
        {
            if (!it.IsString())
            {
                return "Constraint names must be array of string.";
            }

            string cTypeStr = it.GetString();

            if (!(cTypeStr == "gt" || cTypeStr == "lt" || cTypeStr == "eq" || cTypeStr == "fr"))
            {
                return "Constraint type should be one of: 'lt', 'gt' or 'eq'.";
            }
        }

        if (!ensureArray(model, "consVals"))
        {
            return "Constraint rhs values undefined or invalid (expected array of double)";
        }

        const auto &consVals = model["consVals"].GetArray();

        if (consVals.Size() != numCons)
        {
            return "Constraint rhs values size does not correspond to amount of constraints";
        }

        for (const auto &val : consVals)
        {
            if (!val.IsNumber())
            {
                return "Constraints rhs must be array of double.";
            }
        }

        if (!ensureArray(model, "constraints"))
        {
            return "Constraints matrix undefined or invalid (expected array of array of double)";
        }

        const auto &consMat = model["constraints"].GetArray();

        if (consMat.Size() != numCons)
        {
            return "Constraint coef matrix size does not correspond to amount of constraints";
        }

        for (const auto &ar : consMat)
        {
            const char *msg = "Constraints matrix must be array of array of double.";

            if (!ar.IsArray())
            {
                return msg;
            }

            if (ar.GetArray().Size() != numVars)
            {
                return "Constraint coef row size does not correspond to amount of varaibles.";
            }

            for (const auto &val : ar.GetArray())
            {
                if (!val.IsNumber())
                {
                    return msg;
                }
            }
        }

        return "";
    }

    void SolverRouter::createStats(
        Writer<StringBuffer> &w,
        const std::chrono::system_clock::time_point &start,
        const std::chrono::system_clock::time_point &end,
        size_t mem,
        const IntStats *stats)
    {
        const std::chrono::duration<double, std::milli> took = end - start;

        w.StartObject();

        w.Key("iterations");
        if (stats != nullptr && stats->numCuts > 0)
        {
            w.Int(glp_get_prim_iter(GLPKModel) * (stats->numCuts + 1));
        }
        else
        {
            w.Int(glp_get_prim_iter(GLPKModel));
        }

        if (stats != nullptr)
        {
            w.Key("cuts");
            w.Int(stats->numCuts);

            w.Key("branches");
            w.Int(stats->numBranches);
        }

        w.Key("took");
        w.Double(took.count());

        w.Key("started");
        w.Int64(std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count());

        w.Key("finished");
        w.Int64(std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count());

        w.Key("memory");
        w.Int64(mem);

        w.EndObject();
    }

    string SolverRouter::processSimplexCode(int code)
    {
        if (code != 0)
        {
            if (code == GLP_EBOUND)
            {
                return createJsonError("Solver failed due to invalid bounds");
            }
            else if (code == GLP_EFAIL)
            {
                return createJsonError("Solver failed because problem does not have variables/constraints");
            }
            else
            {
                return createJsonError("Solver failed due to internal error");
            }
        }

        return "";
    }

    string SolverRouter::processMipCode(int code)
    {
        if (code != 0)
        {
            return "MIP solver failed due to internal error";
        }

        return "";
    }
}

int main()
{
    using namespace Pistache;
    Address addr(Ipv4::any(), Port(9080));

    auto opts = Http::Endpoint::options().threads(1).flags(Tcp::Options::ReuseAddr);
    Http::Endpoint server(addr);
    server.init(opts);

    lp_pp::rest::SolverRouter router;

    server.setHandler(router.handler());
    server.serve();
}