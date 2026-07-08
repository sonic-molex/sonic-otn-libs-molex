project(otnvirtualhal)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")

# hal_event.h (SAI wire contract) lives in the libsai-otn tree; Json.hpp is
# vendored alongside this component.
include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/../../sai/inc)

add_executable(otn-vhal otn_vhal.cpp otn_alarm_catalog.cpp)

install(TARGETS otn-vhal
        RUNTIME DESTINATION /usr/local/bin)

# Canonical catalog -- shipped to the runtime path otn-vhal reads by default
# (OTN_VHAL_CATALOG_DEFAULT). Single source of truth shared with the SAI listener
# and the device eventd profile (default.json).
install(FILES ${CMAKE_SOURCE_DIR}/../../sai/alarm/otn_alarm_catalog.json
        DESTINATION /usr/share/sonic/otn)
