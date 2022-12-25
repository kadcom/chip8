
enable_language(C)
enable_language(RC)

set(SOURCE_FILES main_win.c render_ddraw.c resource.rc)

add_executable(chip8 WIN32 ${SOURCE_FILES})
target_link_libraries(chip8 ddraw)
set_property(TARGET chip8 PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
