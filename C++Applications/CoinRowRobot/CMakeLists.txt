cmake_minimum_required(VERSION 3.16)
project(CoinRowRobotVisualizer LANGUAGES CXX)

add_executable(coin_row_robot_gui WIN32 coin_row_robot_gui.cpp)
target_compile_features(coin_row_robot_gui PRIVATE cxx_std_17)
target_link_libraries(coin_row_robot_gui PRIVATE gdi32)
