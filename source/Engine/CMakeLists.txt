find_package(SDL2 CONFIG REQUIRED)

add_executable(Engine)
target_sources(Engine PRIVATE
    main.cpp
    RE_Application.cpp
    RE_Render.cpp
)

target_link_libraries(Engine
    PRIVATE
    RedEye_lib
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
