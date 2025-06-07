find_package(SDL3 CONFIG REQUIRED)

add_executable(Engine)
target_sources(Engine PRIVATE
    main.cpp
    RE_Application.cpp
    RE_Render.cpp
)

target_link_libraries(Engine
    PRIVATE
    RedEye_lib
    main PRIVATE SDL3::SDL3
)
