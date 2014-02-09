#include "json.hpp"

namespace dota {
    namespace detail {
        std::string json_to_string(const int64_t& t) {
            return std::to_string(t);
        }
        
        std::string json_to_string(const std::string& t) {
            return "\""+t+"\"";
        }
        
        std::string json_to_string(const std::vector<json_type>& t) {
            std::string fill = "";
            jsonToString v(fill);
            
            std::stringstream ret("");
            ret << "[";
            for (auto it = t.begin(); it != t.end(); ++it) {
                boost::apply_visitor(v, *it);
                ret << fill;
                if (boost::next(it) != t.end()) ret << ",";
            }
            ret << "]";
            return ret.str();
        }
        
        std::string json_to_string(const std::unordered_map<std::string, json_type>& t) {
            std::string fill = "";
            jsonToString v(fill);
            
            std::stringstream ret("");
            ret << "{";
            for (auto it = t.begin(); it != t.end(); ++it) {
                boost::apply_visitor(v, it->second);
                ret << "\"" << it->first << "\":" << fill;
                if (boost::next(it) != t.end()) ret << ",";
            }
            ret << "}";
            return ret.str();
        } 
    }
}