project(libsai-otn)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(TARGET "sai")
set(TEST "sai_test")

### Compiler
### X86 Compiler
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fvisibility=hidden -Wall -Werror")
set(CMAKE_SYSTEM_NAME amd64)

### Require user-specified paths
set(SONIC_PATH "" CACHE PATH "Path to the sonic-buildimage folder (must be specified)")


# Check that paths are specified
if(NOT SONIC_PATH)
    message(FATAL_ERROR "Error: SONIC_PATH must be specified via -DSONIC_PATH=/path/to/sonic-buildimage")
endif()

### include paths
include_directories(${CMAKE_SOURCE_DIR}/inc
                    ${SONIC_PATH}/src/sonic-sairedis/SAI/inc
                    ${SONIC_PATH}/src/sonic-sairedis/SAI/experimental)

#aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_FILES)
# Set a variable to the src directory path
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# --- HAL alarm table codegen ---
# hal_event_listener.cpp's kTable is generated from alarm/otn_alarm_catalog.json
# (the single source of truth) so it is never hand-duplicated. Regenerated
# whenever the catalog or generator changes; the generated .inc lands in the build dir.
set(ALARM_DIR       "${CMAKE_SOURCE_DIR}/alarm")
set(ALARM_CATALOG   "${ALARM_DIR}/otn_alarm_catalog.json")
set(ALARM_TABLE_INC "${CMAKE_BINARY_DIR}/otn_alarm_table.gen.inc")
add_custom_command(
    OUTPUT  ${ALARM_TABLE_INC}
    COMMAND python3 ${ALARM_DIR}/gen_alarm_table.py ${ALARM_CATALOG} ${ALARM_TABLE_INC}
    DEPENDS ${ALARM_CATALOG} ${ALARM_DIR}/gen_alarm_table.py
    COMMENT "Generating HAL alarm table from alarm/otn_alarm_catalog.json")
add_custom_target(gen_alarm_table DEPENDS ${ALARM_TABLE_INC})
include_directories(${CMAKE_BINARY_DIR})

set(DEFAULT_JSON "${SONIC_PATH}/device/molex/x86_64-otn-kvm_x86_64-r0/default.json")

# On-demand only -- NOT part of the default build, so `make` never writes the
# committed device default.json. Run explicitly after editing the catalog:
#   make gen_default_json
add_custom_target(gen_default_json
    COMMAND python3 ${ALARM_DIR}/gen_default_json.py ${ALARM_CATALOG} ${DEFAULT_JSON}
    COMMENT "Regenerating device default.json from the alarm catalog")

# Opt-in drift check (for CI / verify-without-writing): fails if default.json is
# out of sync with the catalog. `make check_default_json`.
add_custom_target(check_default_json
    COMMAND python3 ${ALARM_DIR}/gen_default_json.py --check
            ${ALARM_CATALOG} ${DEFAULT_JSON}
    COMMENT "Checking default.json is in sync with the alarm catalog")

LINK_DIRECTORIES($ENV{LINK_DIR})

add_library(${TARGET} SHARED ${SRC_DIR}/logger.cpp
                             ${SRC_DIR}/dev_util.cpp
                             ${SRC_DIR}/sai_meta_data.cpp
                             ${SRC_DIR}/sai_router_if.cpp
                             ${SRC_DIR}/sai_switch.cpp
                             ${SRC_DIR}/sai_otn_device.cpp
                             ${SRC_DIR}/sai_otn_attenuator.cpp
                             ${SRC_DIR}/sai_otn_oa.cpp
                             ${SRC_DIR}/sai_otn_ocm.cpp
                             ${SRC_DIR}/sai_otn_osc.cpp
                             ${SRC_DIR}/sai_otn_wss.cpp
                             ${SRC_DIR}/sai_otn_otdr.cpp
                             ${SRC_DIR}/sai_otn_otdr_scan_type.cpp
                             ${SRC_DIR}/otdr_static_config.cpp
                             ${SRC_DIR}/sai_adapter.cpp
                             ${SRC_DIR}/sai_adapter_interface.cpp
                             ${SRC_DIR}/virtual_otn_device.cpp
                             ${SRC_DIR}/virtual_otn_dev_mgr.cpp
                             ${SRC_DIR}/hal_event_listener.cpp
                             ${SRC_DIR}/sai.c)


add_dependencies(${TARGET} gen_alarm_table)

target_link_libraries(${TARGET} pthread)

add_executable(${TEST} ${CMAKE_SOURCE_DIR}/test/test.cpp)
target_link_libraries(${TEST} sai)


#debian
set(MAJAR_VERSION 1)
set(MINOR_VERSION 6)
set(PATCH_VERSION 0)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "${MAJAR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_NAME "libsai-otn")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vendor implementation of SONiC SAI API for otn-kvm platform")
set(CPACK_PACKAGE_CONTACK "lu.mao@molex.com")
set(CPACK_DEBIAN_PACKAGE_NAME "libsai")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.31)")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Molex <lu.mao@molex.com>")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_SOURCE_DIR}/doc/changelog)
include(CPack)


install(TARGETS ${TARGET}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        DESTINATION /usr/lib/x86_64-linux-gnu)
install(TARGETS ${TEST}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        DESTINATION /usr/local/bin)

# Ship the alarm catalog in the libsai-otn deb (`make package`). Built from source
# alongside libsai's compiled kTable, so the runtime catalog otn-vhal reads and the
# in-libsai table are always from the same revision (no drift, no stale online deb).
install(FILES ${ALARM_CATALOG}
        DESTINATION /usr/share/sonic/otn)