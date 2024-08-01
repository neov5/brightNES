cmake_minimum_required(VERSION 3.14)
project(brightNES LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE) # for clangd
set(C_STANDARD 99)
set(CXX_STANDARD 17)

set(CMAKE_C_FLAGS_DEBUG "-DNES_DEBUG -DLOG_DEBUG -DLOG_USE_COLOR -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-DLOG_WARN -DLOG_USE_COLOR -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-DNES_DEBUG -DLOG_DEBUG -DLOG_USE_COLOR -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-DLOG_WARN -DLOG_USE_COLOR -O3")

option(SYSTEM_SDL "Use system installed SDL2" 0)

if(SYSTEM_SDL)
    find_package(SDL2 REQUIRED)
    if (NOT TARGET SDL2::SDL2)
        message(FATAL_ERROR "SDL2 not found or not properly installed.")
    endif()
    include_directories(${SDL2_INCLUDE_DIRS})
else()
    add_subdirectory(lib/SDL EXCLUDE_FROM_ALL)
endif()

file(GLOB_RECURSE SOURCES src/*.h src/*.c)
add_executable(brightnes ${SOURCES})
target_link_libraries(brightnes PRIVATE SDL2::SDL2)
