cmake_minimum_required(VERSION 3.7)

project(C-Minecraft C)

set(LIB_FOLDER C:/Programmation/Libraries)



include_directories(${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/include)
include_directories(include/)
include_directories(src/)


link_directories(${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-vc2022)

add_executable(main src/main.c src/glad.c src/window.c src/shader.c)
target_link_libraries(main ${LIB_FOLDER}/glfw-3.3.9.bin.WIN64/lib-vc2022/glfw3.lib)

if( WIN32 )
    set_target_properties(main PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
endif()