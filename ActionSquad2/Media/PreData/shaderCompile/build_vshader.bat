
@if "%~1"=="" goto blank_err

@SET _extension=%~x1
@if NOT "%_extension%"==".hlsl" goto wrong_ext

@echo --- Compiling Vertex Shader ---
@echo Shader to compile: %1
@echo Working directory: %~dp0
@cd %~dp0

@REM save shader name without extension:
@SET shadername=%~n1

@echo.
@echo --- Compiling to VS 3.0 asm ---
fxc /Tvs_3_0 /nologo /Evs_main /Fcbuild/%shadername%.vasm %1

@echo.
@echo --- Assembling shader ---
vsa /nologo /Fobuild/%shadername%.vso build/%shadername%.vasm

@echo.
@echo --- Copy shader to final directory ---
copy /y "build\%shadername%.vso" "../../../Bin/media/shaders"

@goto end_ok

:blank_err

@echo Please specify path of Vertex Shader HLSL to compile!
@echo example: build_vshader.bat C:\shadername.hlsl
@goto EXIT

:wrong_ext
@echo Provided file must have the .hlsl extension!
@goto EXIT

:end_ok

@echo.
@echo --- Compiling Ended ---

:EXIT
@REM should be last label