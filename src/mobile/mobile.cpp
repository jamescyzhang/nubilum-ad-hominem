#include "mobile.hpp"

#include <thread>
#include <string>
#include <iostream>
#include <map>

#include <util/JSON.hpp>

mobile::mobile() : client()
{

}

mobile::mobile(std::string str_addr, std::string str_port) : client(str_addr, str_port)
{

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
            json::JSON pushobj = json::JSON::object{
                    {"id:",        0},
                    {"header",     "msg"},
                    {"importance", 5},
                    {"content",    str},
                    {"notify",     false}
            };
            m_client->send(pushobj.dump());
        }
    }
    return 0;
}
