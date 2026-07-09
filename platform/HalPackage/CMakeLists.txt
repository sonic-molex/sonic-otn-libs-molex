project(SonicLibHal)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_SYSTEM_NAME amd64)

#debian
set(MAJAR_VERSION 1)
set(MINOR_VERSION 2)
set(PATCH_VERSION 0)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "${MAJAR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
set(CPACK_PACKAGE_NAME "libhal-model")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "library hal for sonic")
set(CPACK_PACKAGE_CONTACK "lu.mao@molex.com")
set(CPACK_DEBIAN_PACKAGE_NAME "libhal")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.31)")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Molex <lu.mao@molex.com>")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_SOURCE_DIR}/doc/changelog)
include(CPack)

set(HALAPI_DIR ${CMAKE_SOURCE_DIR}/../HalApiServer/build)
set(HALPLATFORMAPI_DIR ${CMAKE_SOURCE_DIR}/../HalPlatformApiServer/build)
set(OTNVHAL_DIR ${CMAKE_SOURCE_DIR}/../OtnVirtualHal/build)
set(OTNVHAL_SRC ${CMAKE_SOURCE_DIR}/../../sai)
set(CACHE_DIR ${CMAKE_SOURCE_DIR}/cache)
set(FS_DIR ${CMAKE_SOURCE_DIR}/../../shasta/filesystem)
set(FSLIB_DIR ${FS_DIR}/usr/local/lib)
set(FSBIN_DIR ${FS_DIR}/usr/local/bin)


install(FILES ${HALPLATFORMAPI_DIR}/HalPlatformApiServer
              ${HALPLATFORMAPI_DIR}/HalPlatformApiTest
              ${OTNVHAL_DIR}/otn-vhal
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_WRITE GROUP_READ WORLD_EXECUTE WORLD_WRITE WORLD_READ
        DESTINATION /usr/local/bin)

# Canonical alarm catalog read by otn-vhal (OTN_VHAL_CATALOG_DEFAULT).
install(FILES ${OTNVHAL_SRC}/alarm/otn_alarm_catalog.json
        DESTINATION /usr/share/sonic/otn)
