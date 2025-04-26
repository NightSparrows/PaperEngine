@echo off

"%VULKAN_SDK%\Bin\glslc.exe" -g shader.vert -o vert.spv
"%VULKAN_SDK%\Bin\glslc.exe" -g shader.frag -o frag.spv
pause

