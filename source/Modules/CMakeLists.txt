option(ENABLE_OPENGL "Enable OpenGL support" OFF)
option(ENABLE_VULKAN "Enable Vulkan support" ON)

# Ensure mutual exclusivity between Vulkan and OpenGL
if(ENABLE_VULKAN AND ENABLE_OPENGL)
    message(WARNING "Both Vulkan and OpenGL are enabled. Defaulting to OpenGL.")
    set(ENABLE_VULKAN OFF)
elseif(NOT ENABLE_VULKAN AND NOT ENABLE_OPENGL)
    message(WARNING "Neither Vulkan nor OpenGL is enabled. Defaulting to OpenGL.")
    set(ENABLE_OPENGL ON)
endif()

message(STATUS "Configuration Summary:")
message(STATUS "  ENABLE_OPENGL: ${ENABLE_OPENGL}")
message(STATUS "  ENABLE_VULKAN: ${ENABLE_VULKAN}")

find_package(imgui CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
if(ENABLE_OPENGL)
    find_package(GLEW REQUIRED)
else()
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
        $<IF:$<BOOL:${ENABLE_OPENGL}>,Render/OpenGL.ixx,Render/Vulkan.ixx>
)

target_compile_definitions(RedEye_lib PRIVATE IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(RedEye_lib PRIVATE $<IF:$<BOOL:${ENABLE_OPENGL}>,ENABLE_OPENGL,ENABLE_VULKAN>)

target_link_libraries(RedEye_lib PRIVATE
    imgui::imgui
    $<IF:$<TARGET_EXISTS:PhysFS::PhysFS>,PhysFS::PhysFS,PhysFS::PhysFS-static>
    nfd::nfd
    rapidjson
    $<IF:$<BOOL:${ENABLE_OPENGL}>,GLEW::GLEW,Vulkan::Vulkan>
)
