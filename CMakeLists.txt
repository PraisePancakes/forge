cmake_minimum_required(VERSION 3.28)
set(FORGE_VERSION_REGEX "#define FORGE_VERSION_.*[ \t]+(.+)")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/forge/config/version.hpp" FORGE_VERSION REGEX ${FORGE_VERSION_REGEX})
list(TRANSFORM FORGE_VERSION REPLACE ${FORGE_VERSION_REGEX} "\\1")
string(JOIN "." FORGE_VERSION ${FORGE_VERSION})
set(PROJECT_NAME forge)

project(
    ${PROJECT_NAME} 
    VERSION ${FORGE_VERSION} 
    DESCRIPTION "Forging a world one entity at a time. A modern C++ entity-component system (ECS)"
    HOMEPAGE_URL "https://github.com/praispancakes/forge"
    LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS  "*")
message(STATUS  "* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message(STATUS  "*")


add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
target_compile_options(${PROJECT_NAME}
    INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

option(FORGE_BUILD_EXAMPLES "Build Forge examples" OFF)
option(FORGE_BUILD_TESTS "Build Forge tests" ON)

if(FORGE_BUILD_EXAMPLES)
    add_executable(forge_example examples/main.cpp)
    target_link_libraries(forge_example PRIVATE forge)
endif()

if(FORGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()