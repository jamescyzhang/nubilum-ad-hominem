CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(nubilum_ad_hominem)

SET(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(src)

ADD_LIBRARY(nubilum_ad_hominem-json src/util/JSON.hpp src/util/JSON.cpp)

ADD_LIBRARY(nubilum_ad_hominem-comm
        src/net/tcp_client.hpp src/net/tcp_client.cpp
        src/net/tcp_server.hpp src/net/tcp_server.cpp
        src/net/node.hpp src/net/node.cpp
        src/comm/client.hpp src/comm/client.cpp
        src/comm/server.hpp src/comm/server.cpp
        include/net/utils.hpp)

TARGET_LINK_LIBRARIES(nubilum_ad_hominem-comm nubilum_ad_hominem-json)

ADD_EXECUTABLE(nubilum_ad_hominem-server src/server_main.cpp)
ADD_EXECUTABLE(nubilum_ad_hominem-client src/client_main.cpp)

TARGET_LINK_LIBRARIES(nubilum_ad_hominem-server nubilum_ad_hominem-comm)
TARGET_LINK_LIBRARIES(nubilum_ad_hominem-client nubilum_ad_hominem-comm)
