@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -startdir=none -arch=x64 -host_arch=x64 >NUL 2>&1

cmake --build build
