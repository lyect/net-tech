cmake_minimum_required(VERSION 3.24)
project(socks_proxy)

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED OFF)  
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.74.0 REQUIRED system) 

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
    add_executable(socks_proxy main.cpp SocksProxy.cpp SocksProxy.h Logger.cpp Logger.h ClientSession.cpp ClientSession.h)
    target_link_libraries(socks_proxy ${Boost_LIBRARIES})
endif()

