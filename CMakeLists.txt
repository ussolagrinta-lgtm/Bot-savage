cmake_minimum_required(VERSION 3.20)
project(RobRunTool LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)

add_executable(RobRunClient
  client/src/main.cpp
  client/src/driver_io.cpp
  client/src/process_attach.cpp
  client/src/math_util.cpp
  client/src/game_state.cpp
  client/src/features.cpp
  client/src/overlay.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(RobRunClient PRIVATE
  client/include
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${CMAKE_SOURCE_DIR}/shared
)

if(MSVC)
  target_compile_options(RobRunClient PRIVATE /W4 /permissive-)
else()
  target_compile_options(RobRunClient PRIVATE -Wall -Wextra)
endif()

target_link_libraries(RobRunClient PRIVATE d3d11 dxgi dwmapi user32 gdi32)

if(WIN32)
  add_custom_command(TARGET RobRunClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build RobRunMem.sys with WDK — see driver/README.md")
endif()