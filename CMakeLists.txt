cmake_minimum_required(VERSION 3.10)

set(PATCH_VERSION "5" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(join_server VERSION ${PROJECT_VESRION})

find_package(Boost CONFIG REQUIRED)
link_libraries(Boost::boost)

add_executable(join_server main.cpp database.cpp server.cpp commandHelper.cpp)

set_target_properties(join_server PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(join_server
    PRIVATE "${CMAKE_BINARY_DIR}"
)

if (MSVC)
    target_compile_options(join_server PRIVATE
        /W4
    )
else ()
    target_compile_options(join_server PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()

install(TARGETS join_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "NN")
include(CPack)
