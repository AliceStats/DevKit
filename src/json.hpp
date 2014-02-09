#ifndef _DEVKIT_JSON_HPP_
#define _DEVKIT_JSON_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

#include <boost/utility.hpp>
#include <boost/variant.hpp>

namespace dota {
    /** Json property type for pretty printing */
    typedef boost::make_recursive_variant<
        // integer / bool
        int64_t,
        // string
        std::string,                        
        // array 
        std::vector<boost::recursive_variant_>,
        // object
        std::unordered_map<std::string, boost::recursive_variant_> 
    >::type json_type;
    
    namespace detail {
        /** Conversion of int -> string */
        std::string json_to_string(const int64_t& t);
        
        /** Conversion of string -> string */
        std::string json_to_string(const std::string& t);
        
        /** Conversion of bool -> string */
        std::string json_to_string(const bool& t);
        
        /** Conversion of array -> string */
        std::string json_to_string(const std::vector<json_type>& t);
        
        /** Conversion of map -> string */
        std::string json_to_string(const std::unordered_map<std::string, json_type>& t);
    }
    
    /** Visitor to convert a json_type to a string */
    class jsonToString : public boost::static_visitor<> {
        private:
            /** String to fill */
            std::string &toFill;
        public:
            /** Sets internal string as a reference to r */
            jsonToString(std::string &r) : toFill(r) {}

            /** Invoking this does the actual string conversion */
            template <typename T>
            void operator()(const T& t) const {
                toFill = detail::json_to_string(t);
            }
    };
}

#endif  // _DEVKIT_JSON_HPP_