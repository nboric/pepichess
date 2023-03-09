cmake_minimum_required(VERSION 3.23)
project(pepichess C)

set(CMAKE_C_STANDARD 99)

add_executable(pepichess main.c piece_logic.c piece_logic.h common.h common.c move_logic.h move_logic.c init.h init.c prediction_engine.h prediction_engine.c)
