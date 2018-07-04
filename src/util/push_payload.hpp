#ifndef UTIL_PUSH_PAYLOAD_HPP
#define UTIL_PUSH_PAYLOAD_HPP

#include <string>
#include <util/JSON.hpp>

class push_payload
{
public:
    push_payload(std::string str);

    push_payload(json::JSON::object obj);

    push_payload(std::string header, int importance, json::JSON content, bool notify);

    int get_id();

    std::string get_header();

    int get_importance();

    json::JSON get_content();

    int get_timestamp();

    bool should_notify();

    std::string to_str();

private:
    json::JSON m_json;
};

push_payload acknowledge(push_payload incoming);

#endif //UTIL_PUSH_PAYLOAD_HPP
