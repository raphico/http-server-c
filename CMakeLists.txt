cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(codecrafters-http-server LANGUAGES C)

find_package(ZLIB REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.h)

set(CMAKE_C_STANDARD 23) # Enable the C23 standard

add_executable(http-server ${SOURCE_FILES})

target_link_libraries(http-server PRIVATE ZLIB::ZLIB)