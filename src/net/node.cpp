#include "node.hpp"

#include <cstdarg>

namespace net
{
    volatile int node::s_socket_count = 0;
    std::mutex node::s_mtx_count;

    node::node(const log_fn_callback &logger, const settings_flag settings) :
            m_logger(logger),
            m_settings_flags(settings)
    {
        s_mtx_count.lock();
        s_mtx_count.unlock();
    }

    node::~node()
    {
        s_mtx_count.lock();
        s_mtx_count.unlock();
    }

    std::string node::str_format(const std::string format, ...)
    {
        size_t n = format.size() * 2;
        std::unique_ptr<char[]> p_formatted;
        va_list ap;

        while (true)
        {
            p_formatted.reset(new char[n]);
            strcpy(&p_formatted[0], format.c_str());

            va_start(ap, format);
            int i_finaln = vsnprintf(&p_formatted[0], n, format.c_str(), ap);
            va_end(ap);

            if (i_finaln < 0 || i_finaln >= n)
            {
                n += abs(static_cast<int>(i_finaln - n + 1));
            }
            else
            {
                break;
            }
        }

        return std::string(p_formatted.get());
    }
}
