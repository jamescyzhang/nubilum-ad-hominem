#include "mobile.hpp"

#include <thread>
#include <string>
#include <iostream>
#include <map>

#include <util/JSON.hpp>
#include <util/push_payload.hpp>

mobile::mobile() : client()
{
    identity = json::JSON::object{
            {"class", "mobile-device"},
            {"user",  true}
    };
}

mobile::mobile(std::string str_addr, std::string str_port) : client(str_addr, str_port)
{
    identity = json::JSON::object{
            {"class", "mobile-device"},
            {"user",  true}
    };
}

int mobile::run()
{
    client::run();
    while (true)
    {
        std::string str;
        getline(std::cin, str);
        if (str == "!quit")
        {
            m_client->disconnect();
            break;
        }
        else
        {
            push_payload payload("msg", 5, str, false);
            m_client->send(payload.to_str());
        }
    }
    return 0;
}
