#ifndef _DEVKIT_HTTP_HPP_
#define _DEVKIT_HTTP_HPP_

#include <memory>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <ctime>
#include <cstdio>

#include <alice/reader.hpp>
#include <alice/monitor.hpp>
#include <alice/handler.hpp>

#include "http_reply.hpp"
#include "http_request.hpp"
#include "http_request_handler.hpp"
#include "http_mime_type.hpp"
#include "json.hpp"

#include "config.hpp"

namespace dota {
    /** Contains values returned by the STATUS Api call */
    struct game_status {
        /** Default constructor for member initialization */
        game_status() : ticksParsed(0), mode(0), state(0), clock("00:00"), file(""), heroes{"","","","","","","","","",""} {}

        /** Number of messages parsed until now */
        uint32_t ticksParsed;
        /** Game Mode */
        uint32_t mode;
        /** Game State */
        uint32_t state;
        /** In-Game time */
        std::string clock;
        /** Current file open */
        std::string file;
        /** ID's of heroes picked */
        std::vector<json_type> heroes;
    };

    /** Struct for a devkit session */
    class devkit_session {
        public:
            /** Last time session was accessed */
            time_t accessed;
            /** Replay reader */
            reader* r;
            /** delete lock for the reader to give the handler the time to finish */
            std::mutex delLock;
            /** Status struct */
            game_status status;

            devkit_session()
                : accessed(time(NULL)), r(nullptr), delLock(), status(game_status()), heroes{"","","","","","","","","",""} {}

            devkit_session(const devkit_session& s)
                : accessed(s.accessed), r(s.r), delLock(), status(s.status), heroes{"","","","","","","","","",""} {}

            /** binds reader handler to targets */
            void bind() {
                // required for game time
                handlerRegisterCallback(r->getHandler(), msgEntity, "CDOTAGamerulesProxy", devkit_session, handleState);

                // required to get picked heroes
                handlerRegisterCallback(r->getHandler(), msgEntity, "CDOTA_PlayerResource", devkit_session, handlePlayer);
            }

            /** updates game status */
            void update() {
                status.mode = mode;
                status.state = state;
                status.clock = clock;
                status.heroes = heroes;
            }

            /** clears values */
            void clear() {
                status.mode = 0;
                status.state = 0;
                status.clock = "00:00";
                status.heroes = {"","","","","","","","","",""};

                mode = status.mode;
                state = status.state;
                clock = status.clock;
                heroes = status.heroes;
            }
        private:
            /** game status var's to update */
            uint32_t mode;
            uint32_t state;
            std::string clock;
            std::vector<json_type> heroes;

            /** Extract game state */
            void handleState(handlerCbType(msgEntity) msg) {
                auto GameTime = msg->msg->find("DT_DOTAGamerules.m_fGameTime");         // current time
                auto GameStart = msg->msg->find("DT_DOTAGamerules.m_flGameStartTime");  // time when clock reached 0
                auto GameMode = msg->msg->find("DT_DOTAGamerules.m_iGameMode");         // Game Mode
                auto GameState = msg->msg->find("DT_DOTAGamerules.m_nGameState");       // Game State

                float cur = GameTime->second.as<FloatProperty>();
                float start = GameStart->second.as<FloatProperty>();

                if ((start > 1)) {
                    char c[6];
                    float t = cur-start;
                    int mins = std::floor(t/60);
                    int secs = ((int)(t))%60;
                    snprintf(c, 6, "%02d:%02d", mins, secs);
                    c[5] = '\0';
                    clock = c;
                }

                mode = GameMode->second.as<IntProperty>();
                state = GameState->second.as<IntProperty>();
            }

            void handlePlayer(handlerCbType(msgEntity) msg) {
                // This is required because we access the reader outside the monitor.
                // If the reader would be deleted withing the if statement the program
                // would reach an invalid state.
                std::lock_guard<std::mutex> mLock(delLock);

                for (int i = 0; i < 10; ++i) {
                    // get hero entity
                    auto hero = msg->msg->find(std::string("m_hSelectedHero.000")+std::to_string(i));
                    int entityId = (hero->second.as<IntProperty>() & 0x7FF);

                    if (r != nullptr) {
                        gamestate::entityMap &entities = r->getState().getEntities();
                        auto e = entities.find(entityId);
                        if (e != entities.end()) {
                            heroes[i] = e->second->getClassName().substr(16); // only last part
                        }
                    }
                }
            }
    };

    /** Devkit http handler */
    class http_request_handler_devkit : public http_request_handler {
        public:
            /** Possible API methods */
            enum methods {
                LIST         =  0, // list all replays available
                OPEN         =  1, // open a specific replay
                PARSE        =  2, // parse X messages
                CLOSE        =  3, // close message
                STRINGTABLES =  4, // get a list of all stringtables
                STRINGTABLE  =  5, // get contents of a single table
                ENTITIES     =  6, // get a list of all entities
                ENTITY       =  7, // get contents of a single entity
                STATUS       =  8, // returns match status
                RECV         =  9, // returns recvprops
                SEND         = 10  // returns sendprops
            };

            /** Constructor, takes replay directory */
            http_request_handler_devkit(std::string replayDirectory)
                : htdocs(DEVKIT_HTDOCS), replaydir(replayDirectory), count(0) { }

            /** Destructor */
            virtual ~http_request_handler_devkit() { }

            /** handle the http request */
            virtual http_reply handle(http_request req);

            /** Returns session if it exists */
            std::shared_ptr<monitor<devkit_session>> getSession(uint32_t id);
        private:
            /** htdocs folder, set via DEVKIT_HTDOCS define */
            std::string htdocs;
            /** replay folder */
            std::string replaydir;
            /** session count */
            std::atomic<uint32_t> count;
            /** session map */
            std::unordered_map<uint32_t, std::shared_ptr<monitor<devkit_session>>> sessions;
            /** mutex for locking / unlocking the session map */
            std::mutex sessionMutex;

            /** Returns result of LIST API call */
            std::string methodList();
            /** Returns result of OPEN API call */
            std::string methodOpen(std::string arg, uint32_t sId);
            /** Returns result of PARSE API call */
            std::string methodParse(std::string arg, uint32_t sId);
            /** Returns result of CLOSE API call */
            std::string methodClose(uint32_t sId);
            /** Returns result of STRINGTABLES API call */
            std::string methodStringtables(uint32_t sId);
            /** Returns result of ENTITIES API call */
            std::string methodEntities(uint32_t sId);
            /** Returns result of ENTITY API call */
            std::string methodEntity(std::string arg, uint32_t sId);
            /** Returns result of STATUS API call */
            std::string methodStatus(uint32_t sId);
            /** Returns result of SEND API call */
            std::string methodSend(uint32_t sId);
            /** Returns result of RECV API call */
            std::string methodRecv(uint32_t sId);
    };
}

#endif // _DEVKIT_HTTP_HPP_