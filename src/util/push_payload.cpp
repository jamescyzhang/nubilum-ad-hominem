#include "push_payload.hpp"

#include <ctime>
#include <iostream>
#include <util/JSON.hpp>

push_payload::push_payload(std::string str)
{
    std::string err;
    m_json = json::JSON::parse(str, err);
}

push_payload::push_payload(json::JSON::object obj) : m_json(obj)
{
    std::cout << m_json.dump() << std::endl;
}

push_payload::push_payload(std::string header, int importance, json::JSON content, bool notify)
{
    srand(time(NULL));
    m_json = json::JSON::object{
            {"id",        rand()},
            {"header",     header},
            {"importance", importance},
            {"content",    content},
            {"timestamp",  static_cast<int> (std::time(0))},
            {"notify",     false}
    };


}

int push_payload::get_id()
{
    return m_json["id"].int_value();
}

std::string push_payload::get_header()
{
    return m_json["header"].string_value();
}

int push_payload::get_importance()
{
    return m_json["importance"].int_value();
}

json::JSON push_payload::get_content()
{
    return m_json["content"];
}

int push_payload::get_timestamp()
{
    return m_json["timestamp"].int_value();
}

bool push_payload::should_notify()
{
    return m_json["notify"].bool_value();
}

std::string push_payload::to_str()
{
    return m_json.dump();
}

push_payload acknowledge(push_payload incoming)
{
    return push_payload("ack", incoming.get_importance(), json::JSON::object{
            {"recv-id",        incoming.get_id()},
            {"recv-timestamp", incoming.get_timestamp()}
    }, false);
}
