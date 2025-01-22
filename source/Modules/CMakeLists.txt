option(ENABLE_OPENGL "Enable OpenGL support" ON)
option(ENABLE_VULKAN "Enable Vulkan support" ON)

message(STATUS "Configuration Summary:")
message(STATUS "  ENABLE_OPENGL: ${ENABLE_OPENGL}")
message(STATUS "  ENABLE_VULKAN: ${ENABLE_VULKAN}")

find_package(imgui CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
if(ENABLE_OPENGL)
    find_package(GLEW REQUIRED)
endif()
if(ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
endif()

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
        $<IF:$<BOOL:${ENABLE_OPENGL}>,Render/OpenGL.ixx,>
        $<IF:$<BOOL:${ENABLE_VULKAN}>,Render/Vulkan.ixx,>
)

target_compile_definitions(RedEye_lib PRIVATE
    IMGUI_DEFINE_MATH_OPERATORS
    $<IF:$<BOOL:${ENABLE_OPENGL}>,ENABLE_OPENGL,>
    $<IF:$<BOOL:${ENABLE_VULKAN}>,ENABLE_VULKAN,>
)

target_link_libraries(RedEye_lib PRIVATE
    imgui::imgui
    $<IF:$<TARGET_EXISTS:PhysFS::PhysFS>,PhysFS::PhysFS,PhysFS::PhysFS-static>
    nfd::nfd
    rapidjson
    $<IF:$<BOOL:${ENABLE_OPENGL}>,GLEW::GLEW,>
    $<IF:$<BOOL:${ENABLE_VULKAN}>,Vulkan::Vulkan,>
)
