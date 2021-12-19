#pragma once

#include <string>

#include <pistache/router.h>
#include <rapidjson/document.h>
#include <glpk.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace lp_pp::rest
{
    struct IntStats
    {
        int numBranches;
        int numCuts;
    };

    struct SolverRouter : public Pistache::Rest::Router
    {
        SolverRouter();

        void runSolver(const Pistache::Rest::Request &req,
                       Pistache::Http::ResponseWriter res);

        void setModel(const Pistache::Rest::Request &req,
                      Pistache::Http::ResponseWriter res);

        void loadFile(const Pistache::Rest::Request &req,
                      Pistache::Http::ResponseWriter res);

        void runGLPKSimplex(const Pistache::Rest::Request &req,
                            Pistache::Http::ResponseWriter res);

        void runSimplex(const Pistache::Rest::Request &req,
                        Pistache::Http::ResponseWriter res);
        void runBnB(const Pistache::Rest::Request &req,
                    Pistache::Http::ResponseWriter res);
        void runHomory(const Pistache::Rest::Request &req,
                       Pistache::Http::ResponseWriter res);

    private:
        glp_prob *GLPKModel = nullptr;
        std::string filename;

        std::string createJsonError(std::string message, std::string reason = "");

        bool ensureString(const rapidjson::Document &doc, const char *key)
        {
            return doc.HasMember(key) && doc[key].IsString();
        }
        bool ensureArray(const rapidjson::Document &doc, const char *key)
        {
            return doc.HasMember(key) && doc[key].IsArray();
        }

        std::string processSimplexStatus(int code);
        std::string processMipStatus(int code);
        void setHeaders(
            const Pistache::Rest::Request &req,
            Pistache::Http::ResponseWriter &response);

        void jsonToGlpkModel(const rapidjson::Document &json, glp_prob *prob);
        std::string validateModel(const rapidjson::Document &json);
        void createStats(
            rapidjson::Writer<rapidjson::StringBuffer> &w,
            const std::chrono::system_clock::time_point &start,
            const std::chrono::system_clock::time_point &end,
            size_t mem,
            const IntStats *stats = nullptr);

        bool ensureFile();
        std::string processSimplexCode(int code);
        std::string processMipCode(int code);
        std::string launchSimplex(const Pistache::Http::ResponseWriter &res);
        std::string writeSimplexResponse(const rapidjson::Writer<rapidjson::StringBuffer> &w);
    };
}