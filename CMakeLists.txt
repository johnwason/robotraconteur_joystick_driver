cmake_minimum_required(VERSION 3.3)
project(robotraconteur_joystick_driver)

set (CMAKE_CXX_STANDARD 11)

find_package(RobotRaconteur REQUIRED)
find_package(RobotRaconteurCompanion REQUIRED)
find_package(SDL2 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(drekar-launch-process-cpp CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(robotraconteur_joystick_driver
    src/robotraconteur_joystick_driver.cpp
    src/joystick_impl.cpp
    src/joystick_impl.h
)

target_link_libraries(robotraconteur_joystick_driver RobotRaconteurCompanion RobotRaconteurCore SDL2::SDL2 yaml-cpp drekar-launch-process-cpp)
target_compile_definitions(robotraconteur_joystick_driver PRIVATE SDL_MAIN_HANDLED)
if (MSVC)
target_compile_options(robotraconteur_joystick_driver PRIVATE /bigobj)
endif()
