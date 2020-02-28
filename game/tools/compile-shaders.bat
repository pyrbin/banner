@echo off
REM Run from root directory!
if not exist "..\..\build\x64-Debug\game\shaders" mkdir "..\..\build\x64-Debug\game\shaders"

echo "Compiling shaders..."
glslc.exe -fshader-stage=vert ../shaders/shader.vert.glsl -o ../../build/x64-Debug/game/shaders/shader.vert.spv
glslc.exe -fshader-stage=frag ../shaders/shader.frag.glsl -o ../../build/x64-Debug/game/shaders/shader.frag.spv

echo "Done."