cmake_minimum_required(VERSION 3.14)
project(brightNES LANGUAGES C)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE) # for clangd
enable_testing()

# file(GLOB_RECURSE SOURCES src/*.h src/*.c)
add_library(cpu src/cpu.h src/cpu.c)

add_executable(functional_test test/functional.c)
target_include_directories(functional_test PUBLIC src)
target_link_libraries(functional_test cpu)
add_test(NAME functional COMMAND ./functional_test ../test/res/6502_functional_test.bin)
set_property (TEST functional PROPERTY PASS_REGULAR_EXPRESSION "Success")
