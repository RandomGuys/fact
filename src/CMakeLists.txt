add_definitions(-Wall -std=c99 -pthread -Ofast)
find_package(Threads)
find_package(GMP)
add_executable(fact fact_main.c fact.c)
target_link_libraries(fact gmp)

add_executable(test fact.c test.c)
target_link_libraries(test gmp)
