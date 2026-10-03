# Widget style "samhie" (QProxyStyle bọc Kvantum) — build trong stage
# "samhie-style" của recipe (recipes/stages.yml), chỉ file .so vào image.
cmake_minimum_required(VERSION 3.20)
project(samhie-style LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(samhie-style MODULE samhie-style.cpp)
target_link_libraries(samhie-style PRIVATE Qt6::Widgets)
target_compile_options(samhie-style PRIVATE -Wall -Wextra -Werror)
set_target_properties(samhie-style PROPERTIES PREFIX "lib" OUTPUT_NAME "samhie")
