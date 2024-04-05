cmake_minimum_required(VERSION 3.7)

project(C-Minecraft C)

set(LIB_FOLDER C:/Programmation/Libraries)

find_package(OpenGL REQUIRED)

include_directories(${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/include)
include_directories(include/)
include_directories(lib/)
include_directories(src/)



add_executable(main src/main.c src/glad.c src/window.c src/shader.c src/camera.c src/block.c src/chunk.c src/world.c src/player.c src/direction.c src/atlas.c src/generation.c src/gpu.c src/hashtable.c src/stack.c src/fixed_array.c src/queue.c src/frustum.c src/chunk_mesh.c)
set_property(TARGET main PROPERTY C_STANDARD 11)

if( WIN32 )
    link_directories(${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-vc2022)
    target_link_libraries(main ${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3.lib)
    set_target_properties(main PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
    target_compile_definitions(main PRIVATE _CRT_SECURE_NO_WARNINGS) # Removes sprintf warning from cglm
elseif( UNIX )
    target_link_libraries(main m glfw)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -ggdb -Wall -Wextra")
endif()