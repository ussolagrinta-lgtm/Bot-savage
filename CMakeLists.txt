cmake_minimum_required(VERSION 3.20)
project(RobRunTool LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.6)
FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)

# Set WEBVIEW2_SDK to the Microsoft WebView2 SDK root (include/ + lib/).
if(DEFINED ENV{WEBVIEW2_SDK})
  set(WEBVIEW2_SDK $ENV{WEBVIEW2_SDK})
endif()

add_executable(RobRunPort
  client/src/main.cpp
  client/src/driver_io.cpp
  client/src/process_attach.cpp
  client/src/math_util.cpp
  client/src/game_state.cpp
  client/src/world_chain.cpp
  client/src/features.cpp
  client/src/overlay.cpp
  client/src/core_rr_config.cpp
  client/src/status_log.cpp
  client/src/webview_menu.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(RobRunPort PRIVATE
  client/include
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${CMAKE_SOURCE_DIR}/shared
)

if(WEBVIEW2_SDK)
  target_include_directories(RobRunPort PRIVATE "${WEBVIEW2_SDK}/include")
  target_link_directories(RobRunPort PRIVATE "${WEBVIEW2_SDK}/lib/x64")
  target_link_libraries(RobRunPort PRIVATE WebView2LoaderStatic.lib)
endif()

if(MSVC)
  target_compile_options(RobRunPort PRIVATE /W4 /permissive-)
else()
  target_compile_options(RobRunPort PRIVATE -Wall -Wextra)
endif()

target_link_libraries(RobRunPort PRIVATE d3d11 dxgi dwmapi user32 gdi32 ole32 shlwapi)

add_custom_command(TARGET RobRunPort POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/client/resources/menu.html"
          "$<TARGET_FILE_DIR:RobRunPort>/menu.html")

# Optional FF-style supervisor (spawns RobRunPort after game is up).
add_executable(FFSupervisor launcher/ff_supervisor.cpp)
target_link_libraries(FFSupervisor PRIVATE user32 kernel32 shlwapi)

add_custom_command(TARGET FFSupervisor POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/configs/RobRunAIO.ini"
          "$<TARGET_FILE_DIR:FFSupervisor>/RobRunAIO.ini")

# Clean-room entry + static dump artifacts (analysis/DUMP.md).
include("${CMAKE_SOURCE_DIR}/robrun/CMakeLists.txt")