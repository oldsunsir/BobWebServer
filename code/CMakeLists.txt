cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Debug)
set(SERVER_SOURCE_FILES buffer/buffer.cpp 
                        log/log.cpp
                        pool/sqlconnpool.cpp
                        http/httpconn.cpp
                        http/httprequest.cpp
                        http/httpresponse.cpp
                        server/epoller.cpp
                        server/webserver.cpp
                        timer/heaptimer.cpp
                        main.cpp)

add_executable(Server ${SERVER_SOURCE_FILES})   
target_link_directories(Server PUBLIC ${PROJECT_SOURCE_DIR})    
target_link_libraries(Server pthread mysqlclient)