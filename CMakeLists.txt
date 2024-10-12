cmake_minimum_required(VERSION 3.5)
project(robotraconteur_joystick_driver VERSION 0.2.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

find_package(RobotRaconteur REQUIRED)
find_package(RobotRaconteurCompanion REQUIRED)
find_package(SDL2 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(drekar-launch-process-cpp CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME}
    src/robotraconteur_joystick_driver.cpp
    src/joystick_impl.cpp
    src/joystick_impl.h
)

target_link_libraries(${PROJECT_NAME} RobotRaconteurCompanion RobotRaconteurCore SDL2::SDL2 yaml-cpp drekar-launch-process-cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)
if (MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif()

include(GNUInstallDirs)

if (win32)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
    POST_EXCLUDE_REGEXES ".*system32/.*\\.dll")
else()
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

if(WIN32)
set(CPACK_GENERATOR "ZIP")
else()
set(CPACK_GENERATOR "TGZ")
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
set(CPACK_GENERATOR "DEB;${CPACK_GENERATOR}")
endif()

include(CPack)
