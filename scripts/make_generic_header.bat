@echo off
set "in_path=%1"
set "var_name=%2"
set "out_path=%3"

echo /* GENERATED */ > %out_path%
xxd -n %var_name% -i %in_path% >> %out_path%
