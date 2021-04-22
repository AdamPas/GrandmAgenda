cmake_minimum_required(VERSION 3.17)
project(GrandmAgenda C)

set(CMAKE_C_STANDARD 99)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(GrandmAgenda src/main.c)
target_link_libraries(GrandmAgenda PRIVATE Threads::Threads)
