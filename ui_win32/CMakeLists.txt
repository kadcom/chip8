enable_language(C)
enable_language(RC)

set(SOURCE_FILES main_win.c render_ddraw.c debug_view.c)
set(HEADER_FILES diagnostics.h render.h)
set(RESOURCE_FILES resource.rc resource.h)

add_executable(chip8 WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
target_link_libraries(chip8 ddraw comctl32 machine)
set_property(TARGET chip8 PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

source_group(includes FILES ${HEADER_FILES})
source_group(sources FILES ${SOURCE_FILES})
source_group(resources FILES ${RESOURCE_FILES})
