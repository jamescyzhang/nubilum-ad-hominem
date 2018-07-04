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

push_payload::push_payload(std::string header, int importance, std::string content, bool notify)
{
    srand(time(NULL));
    m_json = json::JSON::object{
            {"id:",        rand()},
            {"header",     header},
            {"importance", importance},
            {"content",    content},
            {"timestamp",  static_cast<int> (std::time(0))},
            {"notify",     false}
    };


}

long push_payload::get_id()
{
    return m_json["id"].int_value();
}

std::string push_payload::get_header()
{
    return m_json["header"].dump();
}

int push_payload::get_importance()
{
    return m_json["importance"].int_value();
}

std::string push_payload::get_content()
{
    return m_json["content"].string_value();
}

bool push_payload::should_notify()
{
    return m_json["notify"].bool_value();
}

std::string push_payload::to_str()
{
    return m_json.dump();
}
