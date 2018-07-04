#include "server.hpp"

#include <iostream>
#include <thread>
#include <sys/select.h>

namespace nubilum_ad_hominem
{
    server::server(std::string str_port)
    {
        auto log_printer = [](const std::string &strLogMsg)
        {
            std::cout << strLogMsg << std::endl;
        };
        m_server = new net::tcp_server(log_printer, str_port);
        for (int &m_client : m_clients)
        {
            m_client = 0;
        }
        m_server->start_listen(m_client_socket);
        for (int &m_client : m_clients)
        {
            if (m_client == 0)
            {
                m_client = m_client_socket;
                break;
            }
        }
    }

    int server::run()
    {
        std::thread comm(&server::comm_thread, this);
        comm.join();
        return 0;
    }

    int server::comm_thread()
    {
        while (true)
        {
            FD_ZERO(&m_readfds);
            FD_SET(m_server->m_listen_socket, &m_readfds);
            net::node::socket_fd max_sd = m_server->m_listen_socket;
            for (int i = 0; i < 10; i++)
            {
                net::node::socket_fd sd = m_clients[i];
                if (sd > 0)
                    FD_SET(sd, &m_readfds);
                if (sd > max_sd)
                    max_sd = sd;
            }

            int sk = select(max_sd + 1, &m_readfds, nullptr, nullptr, nullptr);

            if ((sk < 0) && (errno != EINTR))
            {
                printf("select error");
            }

            if (FD_ISSET(m_server->m_listen_socket, &m_readfds))
            {
                m_server->start_listen(m_client_socket);
                for (int &m_client : m_clients)
                {
                    if (m_client == 0)
                    {
                        m_client = m_client_socket;
                        break;
                    }
                }
            }
            for (int &m_client : m_clients)
            {
                int sd = m_client;
                if (FD_ISSET(sd, &m_readfds))
                {
                    char *cmd = (char *) malloc(256 * sizeof(char));

                    if (m_server->receive(sd, cmd, 256 * sizeof(char)) <= 0)
                    {
                        m_server->disconnect(sd);
                        m_client = 0;
                        continue;
                    }
                    std::string str(cmd);
                    if (str == "!quitserver")
                    {
                        m_server->disconnect(m_client_socket);
                        return 0;
                    }
                    else
                    {
                        std::cout << "RECV " << ": " << str << std::endl;
                        m_server->send(sd, "Received message: \"" + str + "\"");
                    }
                }
            }
        }
    }

    server::~server()
    {
        free(m_server);
        m_server = nullptr;
    }
}
