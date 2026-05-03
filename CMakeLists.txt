cmake_minimum_required(VERSION 3.20)
project(polycpp_cors VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(POLYCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(POLYCPP_VARY_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(POLYCPP_SOURCE_DIR "" CACHE PATH "Path to a local polycpp checkout")
set(POLYCPP_VARY_SOURCE_DIR "" CACHE PATH "Path to a local polycpp/vary checkout")

if(NOT POLYCPP_SOURCE_DIR AND EXISTS "/path/to/polycpp/CMakeLists.txt")
    set(POLYCPP_SOURCE_DIR "/path/to/polycpp" CACHE PATH "Path to a local polycpp checkout" FORCE)
endif()

if(POLYCPP_SOURCE_DIR AND EXISTS "${POLYCPP_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(polycpp SOURCE_DIR "${POLYCPP_SOURCE_DIR}")
else()
    FetchContent_Declare(
        polycpp
        GIT_REPOSITORY git@github.com:enricohuang/polycpp.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()
FetchContent_MakeAvailable(polycpp)

if(NOT POLYCPP_VARY_SOURCE_DIR AND EXISTS "polycpp/vary/CMakeLists.txt")
    set(POLYCPP_VARY_SOURCE_DIR "polycpp/vary" CACHE PATH "Path to a local polycpp/vary checkout" FORCE)
endif()

if(POLYCPP_VARY_SOURCE_DIR AND EXISTS "${POLYCPP_VARY_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(polycpp_vary SOURCE_DIR "${POLYCPP_VARY_SOURCE_DIR}")
else()
    FetchContent_Declare(
        polycpp_vary
        GIT_REPOSITORY https://github.com/polycpp/vary.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()
FetchContent_MakeAvailable(polycpp_vary)

add_library(polycpp_cors
    src/cors.cpp
)
target_link_libraries(polycpp_cors PUBLIC polycpp)
target_link_libraries(polycpp_cors PUBLIC polycpp_vary)
target_include_directories(polycpp_cors PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(polycpp::cors ALIAS polycpp_cors)

set(POLYCPP_CORS_BUILD_TESTS_DEFAULT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(POLYCPP_CORS_BUILD_TESTS_DEFAULT ON)
endif()
option(POLYCPP_CORS_BUILD_TESTS "Build tests" ${POLYCPP_CORS_BUILD_TESTS_DEFAULT})
if(POLYCPP_CORS_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(test_smoke tests/test_smoke.cpp)
    target_link_libraries(test_smoke PRIVATE polycpp_cors GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_smoke)
endif()

option(POLYCPP_CORS_BUILD_EXAMPLES "Build examples" OFF)
if(POLYCPP_CORS_BUILD_EXAMPLES)
    file(GLOB POLYCPP_CORS_EXAMPLES CONFIGURE_DEPENDS examples/*.cpp)
    foreach(example_source IN LISTS POLYCPP_CORS_EXAMPLES)
        get_filename_component(example_name "${example_source}" NAME_WE)
        add_executable("${example_name}" "${example_source}")
        target_link_libraries("${example_name}" PRIVATE polycpp_cors)
        set_target_properties("${example_name}" PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )
    endforeach()
endif()
