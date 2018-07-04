#include "client.hpp"

#include <iostream>
#include <thread>
#include <util/push_payload.hpp>

namespace nubilum_ad_hominem
{
    client::client(std::string str_addr, std::string str_port)
    {
        auto log_printer = [](const std::string &msg)
        {
            std::cout << msg << std::endl;
        };

        m_client = new net::tcp_client(log_printer);
        m_client->init_connect(str_addr, str_port);
        identity = json::JSON::object{
                {"class", "generic-client"},
                {"user",  false}
        };
    }

    client::client()
    {
        auto log_printer = [](const std::string &msg)
        {
            std::cout << msg << std::endl;
        };

        m_client = new net::tcp_client(log_printer);
        m_client->init_connect("127.0.0.1", "669");
        identity = json::JSON::object{
                {"class", "generic-client"},
                {"user",  false}
        };
    }

    bool client::init_connect(std::string str_addr, std::string str_port)
    {
        return m_client->disconnect() && m_client->init_connect(str_addr, str_port);
    }

    int client::run()
    {
        m_comm_thread = std::thread(&client::comm_thread, this);
        return 0;
    }

    int client::comm_thread()
    {
        while (true)
        {
            char *cmd = (char *) malloc(256 * sizeof(char));

            if (m_client->receive(cmd, 256 * sizeof(char)) <= 0)
            {
                m_client->disconnect();
                return 0;
            }
            std::string str(cmd);
            std::cout << "RECV" << ": " << str << std::endl;
        }
    }

    void client::ident()
    {
        m_client->send(push_payload("idt", 5, identity, false).to_str());
    }

    client::~client()
    {
        if (m_comm_thread.joinable())
            m_comm_thread.join();
    }

}