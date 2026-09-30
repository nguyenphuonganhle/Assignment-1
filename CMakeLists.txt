cmake_minimum_required(VERSION 3.16)
project(boruvka_mst)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(mst_benchmark
    main.cpp
    mst.cpp
)
target_compile_options(mst_benchmark PRIVATE
    -Wall -Wextra -pedantic
)