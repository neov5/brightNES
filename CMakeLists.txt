cmake_minimum_required(VERSION 3.14)
project(brightNES LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE) # for clangd
enable_testing()

add_subdirectory(lib/SDL2-2.30.5 EXCLUDE_FROM_ALL)
add_subdirectory(lib/log.c EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SOURCES src/*.h src/*.c)
add_executable(main ${SOURCES})
# add_library(cpu src/cpu.h src/cpu.c)
target_link_libraries(main PRIVATE SDL2::SDL2)
target_link_libraries(main PRIVATE logc_shared)

# add_executable(functional_test test/functional.c)
# target_include_directories(functional_test PUBLIC src)
# target_link_libraries(functional_test cpu)
# add_test(NAME functional COMMAND ./functional_test ../test/res/6502_functional_test.bin)
# set_property (TEST functional PROPERTY PASS_REGULAR_EXPRESSION "Success")
