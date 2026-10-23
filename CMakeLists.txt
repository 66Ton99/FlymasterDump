cmake_minimum_required (VERSION 3.20)
project( VarioDump )

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( VarioDump_VERSION_MAJOR 1 )
set( VarioDump_VERSION_MINOR 0 )
set( VarioDump_VERSION_PATCH 0 )

include( scripts/thirdparty.cmake )
include(CTest)
enable_testing()

include_directories("${VarioDump_SOURCE_DIR}")
add_subdirectory( progress )
add_subdirectory( serialPortCom )
add_subdirectory( app )

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VENDOR "Schiffler")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "It's great")
set(CPACK_PACKAGE_VERSION ${VarioDump_VERSION_MAJOR}.${VarioDump_VERSION_MINOR}.${VarioDump_VERSION_PATCH}-${CMAKE_BUILD_TYPE})
set(CPACK_PACKAGE_VERSION_MAJOR ${VarioDump_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VarioDump_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VarioDump_VERSION_PATCH})
include(CPack)
