cmake_minimum_required(VERSION 3.10)
project(ApkaMavsdk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MAVSDK REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        telemetry/TelemetryFrame.h
        core/RingBuffer.h
        telemetry/Ingestor.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE MAVSDK::mavsdk)
