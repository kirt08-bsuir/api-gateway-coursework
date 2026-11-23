cmake_minimum_required(VERSION 3.25)
project(api_gateway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS asio beast)

add_executable(gateway src/main.cpp)
target_link_libraries(gateway PRIVATE fmt::fmt Boost::asio Boost::beast)
target_compile_options(gateway PRIVATE -Wall -Wextra -Wpedantic)