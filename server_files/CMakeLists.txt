cmake_minimum_required(VERSION 3.10)

project(Server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(server_files
    main.c
    server/server.c
    server/handlers.c
    server/utils.c
)

target_include_directories(server_files PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_directories(server_files PRIVATE Threads::Threads)