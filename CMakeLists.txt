cmake_minimum_required(VERSION 3.17)
project(Interactive_Agenda C)

set(CMAKE_C_STANDARD 99)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(Interactive_Agenda main.c)
target_link_libraries(Interactive_Agenda PRIVATE Threads::Threads)
