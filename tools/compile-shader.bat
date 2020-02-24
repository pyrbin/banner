@echo off
REM Run from root directory!
if not exist "build\shaders\" mkdir "build\shaders"

echo "Compiling shaders..."
echo "shaders/main.vert.glsl -> build/shaders/main.vert.spv"
glslc.exe -fshader-stage=vert shaders/main.vert.glsl -o out/build/x64-Debug/shaders/main.vert.spv
echo "shaders/main.frag.glsl -> build/shaders/main.frag.spv"
glslc.exe -fshader-stage=frag shaders/main.frag.glsl -o out/build/x64-Debug/shaders/main.frag.spv

echo "Done."
