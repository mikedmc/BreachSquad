**USAGE**
build_pshader.bat and build_vshader.bat can be called from anywhere passing the complete path to the shader file (.hlsl). 
It will compile to asm and assemble the shader, then copy it in the Bin/media/shaders folder of the game.

Add it as a VS external tool:
command: $(ProjectDir)\Media\PreData\shaderCompile\build_vshader.bat
arguments: $(ItemPath)
initial dir: $(ProjectDir)\Media\PreData\shaderCompile