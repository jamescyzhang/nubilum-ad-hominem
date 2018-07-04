#ifndef UTIL_PUSH_PAYLOAD_HPP
#define UTIL_PUSH_PAYLOAD_HPP

#include <string>
#include <util/JSON.hpp>

class push_payload
{
public:
    push_payload(std::string str);

    push_payload(json::JSON::object obj);

    push_payload(std::string header, int importance, std::string content, bool notify);

    long get_id();

    std::string get_header();

    int get_importance();

    std::string get_content();

    bool should_notify();

    std::string to_str();

private:
    json::JSON m_json;
};


#endif //UTIL_PUSH_PAYLOAD_HPP