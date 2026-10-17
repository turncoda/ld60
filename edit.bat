@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -startdir=none -arch=x64 -host_arch=x64 >NUL 2>&1

neovide src\main.c -- -c "vsplit src/game.c | split src/game.h | nnoremap <leader>r :!.\build.bat && build\Debug\mygame.exe<cr>"
