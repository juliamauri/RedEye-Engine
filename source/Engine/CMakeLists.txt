find_package(SDL2 CONFIG REQUIRED)

add_executable(Engine)
target_sources(Engine PRIVATE
    main.cpp
)

target_link_libraries(Engine
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)