cmake_minimum_required(VERSION 3.7)

project(C-Minecraft C)

set(LIB_FOLDER F:/Programmation/Libraries)


include_directories(${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/include)
include_directories(include/)
include_directories(lib/)
include_directories(src/)

add_executable(main src/main.c src/glad.c src/window.c src/shader.c src/camera.c src/block.c src/chunk.c src/world.c src/player.c src/direction.c
                    src/atlas.c src/gpu.c src/hashtable.c src/stack.c src/fixed_array.c src/queue_og.c src/frustum.c src/chunk_mesh.c src/queue.c)
set_property(TARGET main PROPERTY C_STANDARD 11)

if( WIN32 )
    link_directories(${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-mingw-w64)
    target_link_libraries(main ${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-mingw-w64/glfw3.dll)

    # Copy the DLL to the directory of the executable
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-mingw-w64/glfw3.dll
            $<TARGET_FILE_DIR:main>/glfw3.dll
        COMMENT "Copying glfw3.dll to executable directory"
    )

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Wextra -pthread")
elseif( UNIX )
    target_link_libraries(main m glfw)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -pthread -g3 -ggdb -Wall -Wextra")
endif()