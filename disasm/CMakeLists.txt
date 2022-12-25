add_executable(c8dis main.c)

target_link_libraries(c8dis machine)
set_property(TARGET c8dis PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
