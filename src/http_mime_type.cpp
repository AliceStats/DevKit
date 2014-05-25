#include "http_mime_type.hpp"

namespace dota {
    /** Map of available mime-types coupled with their corresponding file extension */
    std::unordered_map<std::string, std::string> http_mime_type::types =
    {
        // html
        { "htm", "text/html" },
        { "html", "text/html" },

        // pictures
        { "gif", "image/gif" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "png", "image/png" },

        // scripts
        { "js", "text/javascript" },
        { "css", "text/css" },

        // data
        { "json", "application/json" }
    };

    /** Mime type to use if file extension is not known */
    std::string http_mime_type::defType = "application/octet-stream";
}
