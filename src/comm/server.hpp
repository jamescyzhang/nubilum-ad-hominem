#ifndef COMM_SERVER_HPP
#define COMM_SERVER_HPP

#include <string>
#include <vector>
#include <net/tcp_server.hpp>

#define MAX_CLIENTS 30

namespace nubilum_ad_hominem
{
    class server
    {
    public:
        explicit server(std::string str_port);

        ~server();

        int run();
        int comm_thread();

    private:
        net::tcp_server *m_server;
        net::node::socket_fd m_client_socket;
        net::node::socket_fd m_clients[MAX_CLIENTS];
        std::vector<net::node::socket_fd> m_user_clients;

        fd_set m_readfds;

    };
}


#endif //COMM_SERVER_HPP
