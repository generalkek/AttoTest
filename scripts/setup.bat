echo off
call genProj.bat

if [%1]==[] (
cmake --build ../build --config Debug
) else (
cmake --build ../build --config Release
)