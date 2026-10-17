@echo off
set "in_path=%1"
set "out_h_path=%2"

echo #pragma once > %out_h_path%
echo /* GENERATED */ >> %out_h_path%
echo #include ^<SDL3/SDL_rect.h^> >> %out_h_path%
jq -r -f %~dp0\spritesheet_json_filter_h.jq %in_path% >> %out_h_path%
