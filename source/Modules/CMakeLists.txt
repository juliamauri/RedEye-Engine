find_package(imgui CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
find_package(GLEW REQUIRED)

add_library(RedEye_lib STATIC)
target_sources(RedEye_lib
    PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES 
        "RE_GUI.ixx"
        "RE_GUIManager.ixx"
        "RE_FileSystem.ixx"
        "RE_EventSystem.ixx"
        "RE_WindowManager.ixx"
        "RE_Dialogs.ixx"
        "RE_JSON.ixx"

        "Render/RenderAPIHandler.ixx"
        "Render/OpenGL.ixx"
        "Render/Vulkan.ixx")

target_compile_definitions(RedEye_lib PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(RedEye_lib PRIVATE ENABLE_OPENGL)
#target_compile_definitions(RedEye_lib PRIVATE ENABLE_VULKAN)

target_link_libraries(RedEye_lib PRIVATE
    imgui::imgui
    $<IF:$<TARGET_EXISTS:PhysFS::PhysFS>,PhysFS::PhysFS,PhysFS::PhysFS-static>
    nfd::nfd
    rapidjson
    GLEW::GLEW
)