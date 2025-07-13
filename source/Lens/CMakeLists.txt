# Find and link dependencies
find_package(SDL3 CONFIG REQUIRED)
# find_package(RapidJSON CONFIG REQUIRED)

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
        SDL3::SDL3
        # rapidjson
)

# Define the destination directory
set(DESTINATION_DIR ${CMAKE_BINARY_DIR}/source/Lens)

# Create a custom command to copy files
add_custom_command(
    TARGET Lens POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DESTINATION_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/OutputFiles/imgui.ini
        ${DESTINATION_DIR}/project/imgui.ini
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/OutputFiles/template
        ${DESTINATION_DIR}/project/template
    COMMENT "Copying resource files to output directory"
)