# RobRunESP — clean-room client aligned to analysis/DUMP.md (reuses shared rob:: core)

add_executable(RobRunESP
  robrun/src/main.cpp
  robrun/src/cli.cpp
  robrun/src/probe.cpp
  client/src/driver_io.cpp
  client/src/process_attach.cpp
  client/src/math_util.cpp
  client/src/world_chain.cpp
  client/src/features.cpp
  client/src/overlay.cpp
  client/src/core_rr_config.cpp
  client/src/status_log.cpp
  client/src/webview_menu.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(RobRunESP PRIVATE
  robrun/include
  client/include
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${CMAKE_SOURCE_DIR}/shared
)

if(WEBVIEW2_SDK)
  target_include_directories(RobRunESP PRIVATE "${WEBVIEW2_SDK}/include")
  target_link_directories(RobRunESP PRIVATE "${WEBVIEW2_SDK}/lib/x64")
  target_link_libraries(RobRunESP PRIVATE WebView2LoaderStatic.lib)
endif()

if(MSVC)
  target_compile_options(RobRunESP PRIVATE /W4 /permissive-)
else()
  target_compile_options(RobRunESP PRIVATE -Wall -Wextra)
endif()

target_link_libraries(RobRunESP PRIVATE d3d11 dxgi dwmapi user32 gdi32 ole32 shlwapi)

set(_MENU_SRC "${CMAKE_SOURCE_DIR}/analysis/dump/menu.html")
if(NOT EXISTS "${_MENU_SRC}")
  set(_MENU_SRC "${CMAKE_SOURCE_DIR}/client/resources/menu.html")
endif()

add_custom_command(TARGET RobRunESP POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_MENU_SRC}"
          "$<TARGET_FILE_DIR:RobRunESP>/menu.html")

add_custom_command(TARGET RobRunESP POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/configs/CoreRR.cfg.example"
          "$<TARGET_FILE_DIR:RobRunESP>/CoreRR.cfg")