cmake_minimum_required(VERSION 3.25)
project(api_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(fmt CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS asio beast)
find_package(GTest CONFIG REQUIRED)

add_library(gateway_core STATIC
    src/core/router.cpp
)

target_include_directories(gateway_core PUBLIC src)

add_executable(gateway 
    src/main.cpp
    src/net/http_server.cpp
    src/net/upstream_client.cpp
)
target_compile_definitions(gateway PRIVATE BOOST_ASIO_NO_DEPRECATED)
target_link_libraries(gateway PRIVATE gateway_core fmt::fmt Boost::asio Boost::beast)

enable_testing()

add_executable(core_tests
    tests/core/router_test.cpp
)

target_link_libraries(core_tests PRIVATE gateway_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(core_tests)