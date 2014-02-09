#include "devkit_http.hpp"

namespace dota {
    http_reply http_request_handler_devkit::handle(http_request req) {
        // sanitize our url
        if (req.url.empty() || (req.url[0] != '/') || (req.url.find("..") != std::string::npos) )
            return http_reply::getStock(http_reply::bad_request);

        // reply object for this request
        http_reply r;

        // check if this is an api or file request
        if (req.url.substr(1, 3) == "api") {
            // minimum link is /api/00/
            if (req.url.size() < 8)
                return http_reply::getStock(http_reply::bad_request);

            uint32_t method = 0;
            try {
                method = boost::lexical_cast<uint32_t>(req.url.substr(5, 2));
            } catch (std::exception &e) {
                return http_reply::getStock(http_reply::bad_request);
            }

            std::string sessionId = req.getCookie("devkit-session");
            bool gotSession = !sessionId.empty();

            switch (method) {
                case LIST:
                case OPEN:
                case PARSE:
                case CLOSE:
                case STRINGTABLES:
                case STRINGTABLE:
                case ENTITIES:
                case ENTITY:
                case STATUS:
                case RECV:
                case SEND:
                default:
                    return http_reply::getStock(http_reply::bad_request);
                    break;
            }

            r.body = "api";
            r.fields.push_back({"Content-Length", std::to_string(r.body.size())});
            r.fields.push_back({"Connection", "Close"});
        } else {
            // add index.html if path ends in slash
            if (req.url[req.url.size() - 1] == '/')
                req.url += "index.html";

            std::string path = htdocs + req.url;
            std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);

            if (!is)
                return http_reply::getStock(http_reply::not_found);

            r.status = http_reply::ok;
            char buf[4096];

            while (is.read(buf, sizeof(buf)).gcount() > 0)
                r.body.append(buf, is.gcount());

            is.close();

            r.fields.push_back({"Content-Length", std::to_string(r.body.size())});
            r.fields.push_back({"Content-Type", http_mime_type::retrieveFromPath(req.url)});
            r.fields.push_back({"Connection", "Close"});
        }

        return r;
    }
}
