cmake_minimum_required(VERSION 3.0)

project(chip8)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif (MSVC)


include_directories(.)
include_directories(machine)

add_subdirectory(machine)
add_subdirectory(disasm)

if (WIN32)
  add_subdirectory(win32)
endif (WIN32)
