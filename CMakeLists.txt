cmake_minimum_required(VERSION 3.10)
project(gbc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

set(SOURCES
    src/main.cpp
    src/CPU.cpp
    src/PPU.cpp
    src/APU.cpp
    src/MEM.cpp
    src/Timer.cpp
    src/Display.cpp
    src/Joypad.cpp
)

set(HEADERS
    include/CPU.hpp
    include/PPU.hpp
    include/APU.hpp
    include/MEM.hpp
    include/Timer.hpp
    include/Display.hpp
    include/Joypad.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_include_directories(${PROJECT_NAME} PRIVATE 
    include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -lGLEW -lGL -lSDL2
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    -O3
)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")