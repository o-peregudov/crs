@echo off
if EXIST "%1" (

echo Redistributable transfer ...

if EXIST "Release\%2.lib" (
copy "Release\%2.dll" "%1" > nul
copy "Release\%2.lib" "%1\lib" > nul
)

if EXIST "Debug\%2d.lib" (
copy "Debug\%2d.dll" "%1\debug" > nul
copy "Debug\%2d.lib" "%1\lib" > nul
)

)