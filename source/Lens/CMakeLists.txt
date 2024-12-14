# Find and link dependencies
find_package(SDL2 CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_executable(Lens)
target_sources(Lens PRIVATE
    main.cpp
    RL_Application.cpp    
    RL_Input.cpp
    RL_Projects.cpp
    "RL_WindowAndRenderer.cpp"
)

# Compile definitions
target_compile_definitions(Lens PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

# Link libraries
target_link_libraries(Lens PRIVATE
    RedEye_lib
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    nfd::nfd
    rapidjson
)