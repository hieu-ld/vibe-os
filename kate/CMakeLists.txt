# Plugin Kate "samhiekate" (Kate kiểu VS Code của Samhie): terminal làm TAB
# trong vùng editor, thanh menu gọn. Build NGAY TRONG image bởi
# files/scripts/kate-plugin.sh (cùng lý do với style/: Qt lúc build không được
# mới hơn Qt lúc chạy).
cmake_minimum_required(VERSION 3.20)
project(samhie-kate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
# KF6 không có gói gộp "KF6" — mỗi thư viện một gói (như style/)
foreach(lib TextEditor Parts XmlGui CoreAddons WidgetsAddons)
    find_package(KF6${lib} REQUIRED)
endforeach()

add_library(samhiekateplugin MODULE samhie-kate.cpp samhie-kate.qrc)
target_link_libraries(samhiekateplugin PRIVATE
    Qt6::Widgets KF6::TextEditor KF6::Parts KF6::XmlGui KF6::CoreAddons KF6::WidgetsAddons)
target_compile_options(samhiekateplugin PRIVATE -Wall -Wextra -Werror)
set_target_properties(samhiekateplugin PROPERTIES PREFIX "")
