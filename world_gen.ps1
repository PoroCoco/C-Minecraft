rm build/libworld_gen.dll
gcc.exe -shared -o .\build\libworld_gen.dll -fPIC .\src\generation.c -L.\lib\ -I.\lib\ -I.\src