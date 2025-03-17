cmake_minimum_required(VERSION 3.31)
project(matrix)

set(CMAKE_C_STANDARD 11)
find_package(OpenCL REQUIRED)

add_executable(matrix matrix.c compile.c file.c process.c program.c)
target_link_libraries(matrix PRIVATE OpenCL::OpenCL m)
file(COPY matrix.csv kernel.cl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

