cmake_minimum_required(VERSION 3.5)
project(cpu_usage_app)

set(CMAKE_C_STANDARD 11)

find_package(Threads REQUIRED)

add_executable(cpu_usage_app main.c)

target_link_libraries(cpu_usage_app Threads::Threads)
