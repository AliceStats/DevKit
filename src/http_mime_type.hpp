#ifndef _HTTP_MIME_TYPE_HPP_
#define _HTTP_MIME_TYPE_HPP_

#include <string>
#include <unordered_map>

namespace dota {
    /** Class keeping track of different mime types */
    class http_mime_type {
        private:
            /** Map of available mime-types coupled with their corresponding file extension */
            static std::unordered_map<std::string, std::string> types;
            /** Mime type to use if file extension is not known */
            static std::string defType;
        public:
            /** Register a new minetype */
            static void add(std::string&& extension, std::string&& type) {
                http_mime_type::types.emplace(std::move(extension), std::move(type));
            }

            /** Get the type for a specific extension */
            static const std::string& retrieve(const std::string& extension) {
                if (http_mime_type::types.find(extension) == http_mime_type::types.end())
                    return http_mime_type::defType;

                return http_mime_type::types[extension];
            }

            /** Get type for a specific extension extracted from a full path */
            static const std::string& retrieveFromPath(const std::string& path) {
                std::size_t last_slash_pos = path.find_last_of("/");
                std::size_t last_dot_pos   = path.find_last_of(".");
                if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
                    return retrieve(path.substr(last_dot_pos + 1));

                return http_mime_type::defType;
            }
    };
}

#endif // _HTTP_MIME_TYPE_HPP_