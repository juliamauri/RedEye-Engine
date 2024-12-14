find_package(imgui CONFIG REQUIRED)

add_library(windowsM_lib STATIC)
target_sources(windowsM_lib
    PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES RE_Window.ixx RE_WindowsManager.ixx
)

target_compile_definitions(windowsM_lib PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

target_link_libraries(windowsM_lib PRIVATE
    imgui::imgui
)