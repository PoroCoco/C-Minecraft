Remove-Item .\libworld_gen.dll -ErrorAction SilentlyContinue
gcc.exe -shared -o .\libworld_gen.dll -fPIC ..\src\generation.c -L..\lib\ -I..\lib\ -I..\src