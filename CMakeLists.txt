cmake_minimum_required(VERSION 3.0)

project(chip8) 


add_subdirectory(check)
add_subdirectory(adopt)

include_directories(.)

add_subdirectory(log)
add_subdirectory(machine)

include_directories(machine)
add_subdirectory(disasm)

if (WIN32)
add_subdirectory(win32)
endif (WIN32)
