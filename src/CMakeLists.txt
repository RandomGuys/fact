add_definitions(-Wall -std=c99 -pthread -Ofast -g)
find_package(Threads)
find_package(GMP)
add_executable(fact fact_main.c fact.c fact_superspeed.c)
target_link_libraries(fact ${CMAKE_THREAD_LIBS_INIT} gmp m)

add_executable(test fact.c test.c)
target_link_libraries(test ${CMAKE_THREAD_LIBS_INIT} gmp m)
