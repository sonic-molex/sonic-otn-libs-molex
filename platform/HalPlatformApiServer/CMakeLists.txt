project(halplatformapi)
cmake_minimum_required(VERSION 3.5)
set(SERVER "HalPlatformApiServer")
set(CLIENT "HalPlatformApiTest")

### Compiler
### X86 Compiler
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")

### thrift
include_directories(/usr/include/boost
                    /usr/local/include/boost)


LINK_DIRECTORIES($ENV{LINK_DIR})
add_executable (${SERVER}       Convert.cpp
                                ApiHelper.cpp
                                HalPlatformApi.cpp
                                HalPlatformApi_server.cpp
                                Json.hpp
                                virtual_device.h      
                                virtual_platform.h
                                logger.cpp)

target_link_libraries (${SERVER} pthread thrift)


add_executable (${CLIENT}       HalPlatformApi.cpp
                                HalPlatformApiTest.cpp)

target_link_libraries (${CLIENT} thrift)