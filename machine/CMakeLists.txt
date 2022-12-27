set(HEADER_FILES chip8.h chip8_errors.h common.h)
set(INTERNAL_HEADER_FILES chip8_internal.h)
set(SOURCE_FILES chip8.c routines.c)

add_library(machine ${SOURCE_FILES})
set_property(TARGET machine PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

source_group(inc FILES ${HEADER_FILES})
source_group(int FILES ${INTERNAL_HEADER_FILES})
source_group(sources FILES ${SOURCE_FILES})
