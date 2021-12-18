#pragma once

#include <string>

#include <pistache/router.h>
#include <rapidjson/document.h>
#include <glpk.h>

namespace lp_pp::rest
{
    struct SolverRouter : public Pistache::Rest::Router
    {
        SolverRouter();

        void runGlpk(const Pistache::Rest::Request &req,
                     Pistache::Http::ResponseWriter res);

        void setModel(const Pistache::Rest::Request &req,
                      Pistache::Http::ResponseWriter res);

        void loadFile(const Pistache::Rest::Request &req,
                      Pistache::Http::ResponseWriter res);

        void runSimplex(const Pistache::Rest::Request &req,
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
    };
}