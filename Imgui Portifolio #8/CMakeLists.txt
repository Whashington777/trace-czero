cmake_minimum_required(VERSION 3.20)
project(portfolio8_linux LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(PORTFOLIO_BUILD_PREVIEW "Build the standalone SDL2/OpenGL preview" ON)
option(PORTFOLIO_32BIT "Build for the 32-bit GoldSrc ABI (requires matching dev libraries)" OFF)
if(PORTFOLIO_32BIT)
  add_compile_options(-m32)
  add_link_options(-m32)
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui-current")
add_library(portfolio_gui SHARED
 framework/app/gui.cpp framework/helpers/fonts.cpp framework/helpers/glow.cpp
 framework/helpers/icons.cpp framework/helpers/settings_panel.cpp framework/helpers/widgets.cpp
 framework/helpers/window_drag.cpp framework/platform/glow.cpp framework/platform/screenshot.cpp
 framework/pages/aim_page.cpp framework/pages/visuals_page.cpp
 framework/pages/config_page.cpp framework/pages/settings_page.cpp
 framework/data/fontello_data.cpp framework/data/fonts_data.cpp framework/data/logo_data.cpp
 ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp
 ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp)
set_target_properties(portfolio_gui PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(portfolio_gui PUBLIC framework framework/data framework/helpers
 ${IMGUI_DIR} ${IMGUI_DIR}/backends ${IMGUI_DIR}/misc/freetype thirdparty/stb)
target_compile_definitions(portfolio_gui PUBLIC IMGUI_DEFINE_MATH_OPERATORS IMGUI_ENABLE_FREETYPE)
target_link_options(portfolio_gui PRIVATE -Wl,--no-undefined)
target_link_libraries(portfolio_gui PUBLIC PkgConfig::FREETYPE GL)
if(PORTFOLIO_BUILD_PREVIEW)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
add_executable(portfolio8 framework/platform/main_sdl.cpp
 ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp)
target_link_libraries(portfolio8 PRIVATE portfolio_gui PkgConfig::SDL2 GL)
endif()
include(CTest)
if(BUILD_TESTING)
  add_executable(portfolio_abi_smoke tests/abi_smoke.cpp)
  target_link_libraries(portfolio_abi_smoke PRIVATE portfolio_gui)
  add_test(NAME gui_abi COMMAND portfolio_abi_smoke)
  if(PORTFOLIO_BUILD_PREVIEW)
    add_test(NAME gui_input COMMAND portfolio8 --self-test)
  endif()
endif()
