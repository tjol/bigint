cmake_minimum_required(VERSION 3.2)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(bigint C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic")

add_library(bigint STATIC bigint.c)
add_executable(bigint_dc bigint_dc.c)
target_link_libraries(bigint_dc bigint)

find_package(Criterion)

if(CRITERION_FOUND)
    add_executable(bigint_test test.c)
    target_link_libraries(bigint_test ${CRITERION_LIBRARIES} bigint)
    target_include_directories(bigint_test PRIVATE ${CRITERION_INCLUDE_DIRS})

    enable_testing()
    add_test(bigint_test bigint_test)
endif()
