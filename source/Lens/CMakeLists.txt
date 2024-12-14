# Find and link dependencies
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(PhysFS CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_executable(Lens)
target_sources(Lens PRIVATE
    main.cpp
    RL_Application.cpp
    RL_FileSystem.cpp
    RL_Input.cpp
    RL_Projects.cpp
    "RL_WindowAndRenderer.cpp"
)
target_sources(Lens PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    "RL_Window.ixx"
    "RL_WindowsManager.ixx"
)

# Compile definitions
target_compile_definitions(Lens PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

# Link libraries
target_link_libraries(Lens PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    imgui::imgui
    nfd::nfd
    $<IF:$<TARGET_EXISTS:PhysFS::PhysFS>,PhysFS::PhysFS,PhysFS::PhysFS-static>
    rapidjson
)