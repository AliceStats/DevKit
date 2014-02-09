#ifndef _HTTP_REQUEST_PARSER_HPP_
#define _HTTP_REQUEST_PARSER_HPP_

#include <string>
#include <memory>
#include <http_parser.h>

#include "http_request.hpp"

namespace dota {
    /** interface to the http_parser C library */
    class http_parser_interface {
         public:
            /** Parsing status */
            enum result_t {
                good,           // done and okay
                bad,            // something went wrong
                indeterminate   // we are between states, need more data
            } result;

            /** Constructor initializing the underlying C parser */
            http_parser_interface() : result(indeterminate), settings(), parser(), req(), parsingValue(false), field(""),
                value("")
            {
                // set callback functions
                settings.on_message_begin = 0;
                settings.on_status = 0;

                settings.on_url = http_parser_interface::CbUrl;
                settings.on_header_field = http_parser_interface::CbField;
                settings.on_header_value = http_parser_interface::CbValue;
                settings.on_headers_complete = http_parser_interface::CbHeaderComplete;
                settings.on_body = http_parser_interface::CbBody;
                settings.on_message_complete = http_parser_interface::CbComplete;

                // initialize the http parser
                http_parser_init(&parser, HTTP_REQUEST);
                parser.data = this;
            }

            /** Invokes the parser */
            result_t parse(const char* data, std::size_t transferred) {
                std::size_t parsed = http_parser_execute(&parser, &settings, data, transferred);

                if (parsed != transferred)
                    result = bad;

                return result;
            }

            /** Return the http_request */
            http_request getRequest() {
                result = indeterminate;
                return std::move(req);
            }

            /** Callback for the url requested */
            static int CbUrl(http_parser *p, const char *at, size_t len) {
                http_parser_interface *t = static_cast<http_parser_interface*>(p->data);
                t->req.url = std::string(at, len);
                return 0;
            }

            /** Callback for field id */
            static int CbField(http_parser *p, const char *at, size_t len) {
                http_parser_interface *t = static_cast<http_parser_interface*>(p->data);

                if (t->parsingValue) {
                    t->applyValues();
                    t->field.append(at, len);
                } else {
                    t->field.append(at, len);
                }

                return 0;
            }

            /** Callback for field value */
            static int CbValue(http_parser *p, const char *at, size_t len) {
                http_parser_interface *t = static_cast<http_parser_interface*>(p->data);

                t->value.append(at, len);
                t->parsingValue = true;

                return 0;
            }

            /** Callback after header was read */
            static int CbHeaderComplete(http_parser *p) {
                http_parser_interface *t = static_cast<http_parser_interface*>(p->data);
                t->applyValues();
                return 0;
            }

            /** Callback after body was read */
            static int CbBody(http_parser *p, const char *at, size_t len) {
                http_parser_interface *t = static_cast<http_parser_interface*>(p->data);
                t->req.body.append(at, len);
                return 0;
            }

            /** Callback after parsing is finished */
            static int CbComplete(http_parser *p) {
                http_parser_interface *t = static_cast<http_parser_interface*>(p->data);
                t->result = good;
                return 0;
            }
        private:
            /** Settings for the http parser */
            http_parser_settings settings;
            /** http parser struct */
            http_parser parser;
            /** http request to fill */
            http_request req;

            /** State for switching between lines and values */
            bool parsingValue;
            /** Current field */
            std::string field;
            /** Current value */
            std::string value;

            /** Add current values to request */
            void applyValues() {
                // add to map
                req.fields.emplace(std::move(field), std::move(value));

                // reset status
                field = "";
                value = "";
                parsingValue = false;
            }
    };
}

#endif // _HTTP_REQUEST_PARSER_HPP_