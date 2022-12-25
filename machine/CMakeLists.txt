set(HEADER_FILES chip8.h chip8_errors.h chip8_internal.h common.h)
set(SOURCE_FILES chip8.c)

add_library(machine ${SOURCE_FILES})
