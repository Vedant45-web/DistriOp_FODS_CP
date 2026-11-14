#pragma once
#include <string>
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>

inline std::string jsonEscape(const std::string& s) {
    std::ostringstream o;
    for (unsigned char c : s) {
        switch(c) { case '"': o << "\\\""; break; case '\\': o << "\\\\"; break; case '\n': o << "\\n"; break; case '\r': o << "\\r"; break; case '\t': o << "\\t"; break; default: if(c < 32) o << ' '; else o << c; }
    }
    return o.str();
}
inline std::string getString(const std::string& body, const std::string& key, const std::string& fallback="") {
    std::regex r("\\\""+key+"\\\"\\s*:\\s*\\\"([^\\\"]*)\\\""); std::smatch m;
    return std::regex_search(body,m,r) ? m[1].str() : fallback;
}
inline double getNumber(const std::string& body, const std::string& key, double fallback=0) {
    std::regex r("\\\""+key+"\\\"\\s*:\\s*(-?[0-9]+(?:\\.[0-9]+)?)"); std::smatch m;
    if(std::regex_search(body,m,r)) {
        try { return std::stod(m[1].str()); } catch(...) { return fallback; }
    }
    std::regex rb("\\\""+key+"\\\"\\s*:\\s*(true|false)");
    if(std::regex_search(body,m,rb)) {
        return m[1].str() == "true" ? 1.0 : 0.0;
    }
    return fallback;
}
inline std::string number(double v) { std::ostringstream o; o << std::fixed << std::setprecision(2) << v; return o.str(); }
