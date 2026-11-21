cmake_minimum_required(VERSION 3.24)
project(SavageCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "SavageCore targets Windows (Win32 + D3D11). Configure on Windows with MSVC.")
endif()

option(SAVAGE_BUILD_DLL "Build SavageModule internal-style DLL" ON)
option(SAVAGE_BUILD_OVERLAY "Build SavageOverlay external overlay EXE" ON)

include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.5
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG master
)
FetchContent_MakeAvailable(minhook)

add_library(savage_imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)
target_include_directories(savage_imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)
target_compile_definitions(savage_imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_link_libraries(savage_imgui PUBLIC d3d11 dxgi)

add_library(minhook STATIC
  ${minhook_SOURCE_DIR}/src/buffer.c
  ${minhook_SOURCE_DIR}/src/hook.c
  ${minhook_SOURCE_DIR}/src/trampoline.c
  ${minhook_SOURCE_DIR}/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC ${minhook_SOURCE_DIR}/include)
if(MSVC)
  target_compile_options(minhook PRIVATE /wd4100 /wd4201)
endif()

add_library(savage_core STATIC
  src/core/logger.cpp
  src/config/game_profile.cpp
  src/platform/win32_util.cpp
  src/memory/process_memory.cpp
  src/memory/pattern_scan.cpp
  src/hooks/minhook_scope.cpp
  src/hooks/present_hook.cpp
  src/render/d3d11_context.cpp
  src/ui/imgui_layer.cpp
  src/sync/state_hub.cpp
)
target_include_directories(savage_core PUBLIC include)
target_link_libraries(savage_core PUBLIC minhook savage_imgui)
target_compile_definitions(savage_core PUBLIC UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(savage_core PUBLIC user32 gdi32 advapi32 d3d11 dxgi)

if(SAVAGE_BUILD_OVERLAY)
  add_executable(SavageOverlay WIN32
    src/apps/overlay_main.cpp
    src/overlay/external_overlay.cpp
  )
  target_link_libraries(SavageOverlay PRIVATE savage_core)
endif()

if(SAVAGE_BUILD_DLL)
  add_library(SavageModule SHARED
    src/apps/module_entry.cpp
  )
  target_link_libraries(SavageModule PRIVATE savage_core)
endif()