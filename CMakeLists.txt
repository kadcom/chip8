cmake_minimum_required(VERSION 3.0)

project(chip8) 


add_subdirectory(log)

add_subdirectory(check)
add_subdirectory(machine)

if (WIN32)
add_subdirectory(win32)
endif (WIN32)
