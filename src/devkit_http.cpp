#include <bitset>
#include <dirent.h>

#include <boost/lexical_cast.hpp>

#include "json.hpp"
#include "devkit_http.hpp"

namespace dota {
    /** Returns json string of succesful request */
    std::string retOk(json_type t) {
        std::string result;
        jsonToString v(result);
        boost::apply_visitor( v, t );

        return std::string("{\"success\":1, \"data\":"+result+"}");
    }

    /** Returns json string of unseccesful request */
    std::string retFail(std::string msg) {
        return std::string("{\"success\":0, \"data\":\""+msg+"\"}");
    }

    http_reply http_request_handler_devkit::handle(http_request req) {
        // sanitize our url
        if (req.url.empty() || (req.url[0] != '/') || (req.url.find("..") != std::string::npos) )
            return http_reply::getStock(http_reply::bad_request);

        // reply object for this request
        http_reply r;
        r.status = http_reply::ok;

        // check if this is an api or file request
        if (req.url.substr(1, 3) == "api") {
            // minimum link is /api/00/
            if (req.url.size() < 8)
                return http_reply::getStock(http_reply::bad_request);

            // get request method
            uint32_t method = 0;
            try {
                method = boost::lexical_cast<uint32_t>(req.url.substr(5, 2));
            } catch (std::exception &e) {
                return http_reply::getStock(http_reply::bad_request);
            }

            // get request session
            uint32_t session;
            std::string sessionId = req.getCookie("devkit-session");
            if (sessionId.empty()) {
                session = ++count;
                r.fields.push_back({"Set-Cookie", std::string("devkit-session=")+std::to_string(session)+"; Path=/; HttpOnly;"});
            } else {
                try {
                    session = boost::lexical_cast<uint32_t>(sessionId);
                } catch (std::exception &e) {
                    return http_reply::getStock(http_reply::bad_request);
                }
            }

            // check if sessions exists
            sessionMutex.lock();
            auto it = sessions.find(session);
            if (it == sessions.end()) {
                sessions[session] = std::shared_ptr<monitor<devkit_session>> (new monitor<devkit_session>(devkit_session{}));
            } else {
                (*it->second)([](devkit_session &s){
                    s.accessed = time(NULL);
                });
            }
            sessionMutex.unlock();

            switch (method) {
                case LIST:
                    r.body = methodList();
                    break;
                case OPEN:
                    r.body = methodOpen(req.url.substr(8), session);
                    break;
                case PARSE:
                    r.body = methodParse(req.url.substr(8), session);
                    break;
                case CLOSE:
                    r.body = methodClose(session);
                    break;
                case STRINGTABLES:
                    r.body = methodStringtables(session);
                    break;
                case STRINGTABLE:
                    // we need to make those available using a javascript HEX editor, but at this point it's overkill
                    r.body = std::string("{\"success\":0, \"data\":\"Stringtable loading is not supported at the moment\"");
                    break;
                case ENTITIES:
                    r.body = methodEntities(session);
                    break;
                case ENTITY:
                    r.body = methodEntity(req.url.substr(8), session);
                    break;
                case STATUS:
                    r.body = methodStatus(session);
                    break;
                case RECV:
                    r.body = methodRecv(session);
                    break;
                case SEND:
                    r.body = methodSend(session);
                    break;
                default:
                    return http_reply::getStock(http_reply::bad_request);
                    break;
            }

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

    std::shared_ptr<monitor<devkit_session>> http_request_handler_devkit::getSession(uint32_t id) {
        std::lock_guard<std::mutex> mLock(sessionMutex);

        auto it = sessions.find(id);
        if (it == sessions.end())
            return nullptr; // just return null in case we don't have the session

        return it->second;
    }

    std::string http_request_handler_devkit::methodList() {
        DIR *dir;
        struct dirent *entry;
        std::vector<json_type> entries;

        if ((dir = opendir(replaydir.c_str())) == NULL)
            return retFail("Failed to open directory");

        while ((entry = readdir (dir)) != NULL) {
            std::string e(entry->d_name);

            // don't add the current and prev dir
            if (e.substr(0,1).compare(".") == 0)
                continue;

            entries.push_back(std::move(e));
        }

        closedir(dir);

        return retOk(entries);
    }

    std::string http_request_handler_devkit::methodOpen(std::string arg, uint32_t sId) {
        // replay file
        std::string file = replaydir+arg;

        // session
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            // open a new replay
            (*session)([=](devkit_session &s) {
                std::lock_guard<std::mutex> mLock(s.delLock);

                if (s.r) {
                    delete s.r;
                    s.clear();
                }

                try {
                    s.r = new reader(file);
                    s.status = game_status(); // reset current status
                    s.status.file = arg;
                    s.bind();
                } catch (std::exception &e) {
                    // race condition if reader can't be opened here and we don't catch it
                    s.r = nullptr;
                    throw e;
                }
            }).get();

            return retOk(std::string("Replay Opened"));
        } catch (std::exception &e) {
            return retFail(std::string("Failed to open replay with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodParse(std::string arg, uint32_t sId) {
        // number of entities to parse
        uint32_t num = 0;

        try {
            if (!arg.empty())
                num = boost::lexical_cast<uint32_t>(arg);

            auto session = getSession(sId);
            if (!session)
                return retFail(std::string("No session active"));

            (*session)([=](devkit_session &s) {
                // don't parse if there is no reader
                if (s.r) {
                    for (uint32_t i = 0; i < num; ++i) {
                        s.r->readMessage();
                    }
                    s.status.ticksParsed += num;
                    s.update();
                }
            });

            return retOk(std::string("Parsing..."));
        } catch (std::exception &e) {
            return retFail(std::string("Failed to parse replay with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodClose(uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            (*session)([=](devkit_session &s) {
                std::lock_guard<std::mutex> mLock(s.delLock);

                if (s.r) {
                    delete s.r;
                    s.r = nullptr;
                    s.clear();
                }
            });

            return retOk(std::string("Replay Closed"));
        } catch (std::exception &e) {
            return retFail(std::string("Failed to close replay with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodStringtables(uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            auto ent = (*session)([=](devkit_session &s) {
                std::unordered_map<std::string, json_type> entries;

                // if there is no reader, this function returns an empty set
                if (!s.r)
                    return entries;

                gamestate::stringtableMap &tables = s.r->getState().getStringtables();
                for (auto &tbl : tables) {
                    std::vector<json_type> sub;

                    for (auto &s : tbl.value) {
                        sub.push_back(s.key);
                    }

                    entries[tbl.key] = sub;
                }

                return entries;
            });

            return retOk(ent.get());
        } catch (std::exception &e) {
            return retFail(std::string("Failed to parse replay with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodEntities(uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            auto ent = (*session)([=](devkit_session &s) {
                std::vector<json_type> entries;

                // returns an empty set if no replay is opened
                if (!s.r)
                    return entries;

                gamestate::entityMap &entities = s.r->getState().getEntities();
                for (auto &ent : entities) {
                    std::vector<json_type> sub;
                    sub.push_back(ent.first);
                    sub.push_back(ent.second->getClassName());
                    entries.push_back(std::move(sub));
                }

                return entries;
            });

            return retOk(ent.get());
        } catch (std::exception &e) {
            return retFail(std::string("Failed to gather entities with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodEntity(std::string arg, uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        // ID of entity to get
        uint32_t id = 0;

        try {
            if (!arg.empty())
                id = boost::lexical_cast<uint32_t>(arg);
            else
                return retFail("No entity specified");

            auto ret = (*session)([=](devkit_session &s) {
                std::vector<json_type> entries;

                if (!s.r)
                    return entries;

                gamestate::entityMap &entities = s.r->getState().getEntities();

                auto e = entities.find(id);
                if (e != entities.end()) {
                    for (auto it : *(e->second)) {
                        std::unordered_map<std::string, json_type> entry;
                        sendprop *p = it.second.getSendprop();

                        entry["name"] = it.second.getName();
                        entry["value"] = it.second.asString();
                        entry["type"] = it.second.getType();
                        entry["flags"] = p->getFlags();

                        entries.push_back(entry);
                    }
                }

                return entries;
            });

            return retOk(ret.get());
        } catch (std::exception &e) {
            return retFail(std::string("Failed to get entity with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodStatus(uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            auto ret = (*session)([=](devkit_session &s) {
                std::unordered_map<std::string, json_type> entries;

                if (!s.r)
                    return entries;

                entries["ticks"] = s.status.ticksParsed;
                entries["time"] = s.status.clock;
                entries["picks"] = s.status.heroes;
                entries["file"] = s.status.file;
                entries["state"] = s.status.state;
                entries["mode"] = s.status.mode;

                return entries;
            });

            return retOk(ret.get());
        } catch (std::exception &e) {
            return retFail(std::string("Failed to get status with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodSend(uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            auto ret = (*session)([=](devkit_session &s) {
                std::unordered_map<std::string, json_type> entries;

                if (!s.r)
                    return entries;

                gamestate::sendtableMap &tbls = s.r->getState().getSendtables();

                for (auto &it : tbls) {
                    std::vector<json_type> props;
                    for (auto &t : it.value) {
                        std::unordered_map<std::string, json_type> entry;

                        entry["type"] = t.value->getType();
                        entry["name"] = t.value->getName();
                        entry["netname"] = t.value->getNetname();
                        entry["flags"] = t.value->getFlags();
                        entry["priority"] = t.value->getPriority();
                        entry["classname"] = t.value->getClassname();
                        entry["elements"] = t.value->getElements();
                        entry["min"] = t.value->getLowVal();
                        entry["max"] = t.value->getHighVal();
                        entry["bits"] = t.value->getBits();

                        props.push_back(entry);
                    }
                    entries[it.value.getName()] = props;
                }

                return entries;
            });

            return retOk(ret.get());
        } catch (std::exception &e) {
            return retFail(std::string("Failed to get sendtables with exception: ")+e.what());
        }
    }

    std::string http_request_handler_devkit::methodRecv(uint32_t sId) {
        auto session = getSession(sId);
        if (!session)
            return retFail(std::string("No session active"));

        try {
            auto ret = (*session)([=](devkit_session &s) {
                std::unordered_map<std::string, json_type> entries;

                if (!s.r)
                    return entries;

                gamestate::flatMap &tbls = s.r->getState().getFlattables();

                for (auto &it : tbls) {
                    std::vector<json_type> props;
                    for (auto &t : it.second.properties) {
                        std::unordered_map<std::string, json_type> entry;
                        std::bitset<32> flagset(t->getFlags());

                        entry["type"] = t->getType();
                        entry["name"] = t->getName();
                        entry["netname"] = t->getNetname();
                        entry["flags"] = flagset.to_string();
                        entry["priority"] = t->getPriority();
                        entry["classname"] = t->getClassname();
                        entry["elements"] = t->getElements();
                        entry["min"] = t->getLowVal();
                        entry["max"] = t->getHighVal();
                        entry["bits"] = t->getBits();

                        props.push_back(entry);
                    }
                    entries[it.second.name] = props;
                }

                return entries;
            });

            return retOk(ret.get());
        } catch (std::exception &e) {
            return retFail(std::string("Failed to get recvtables with exception: ")+e.what());
        }
    }
}