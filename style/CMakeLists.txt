# Widget style "samhie" (QProxyStyle bọc Kvantum) — build NGAY TRONG image bởi
# files/scripts/samhie-style.sh, chỉ file .so ở lại image.
cmake_minimum_required(VERSION 3.20)
project(samhie-style LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
# KPageWidgetModel::item() — đổi icon một trang hộp thoại cài đặt (mục 36)
find_package(KF6WidgetsAddons REQUIRED)

add_library(samhie-style MODULE samhie-style.cpp)
target_link_libraries(samhie-style PRIVATE Qt6::Widgets KF6::WidgetsAddons)
target_compile_options(samhie-style PRIVATE -Wall -Wextra -Werror)
set_target_properties(samhie-style PROPERTIES PREFIX "lib" OUTPUT_NAME "samhie")
