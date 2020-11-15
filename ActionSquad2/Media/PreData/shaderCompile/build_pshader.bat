
@if "%~1"=="" goto blank_err

@SET _extension=%~x1
@if NOT "%_extension%"==".hlsl" goto wrong_ext

@echo --- Compiling Pixel Shader ---
@echo Shader to compile: %1
@echo Working directory: %~dp0
@cd %~dp0

@REM save shader name without extension:
@SET shadername=%~n1

@echo.
@echo --- Compiling to PS 3.0 asm ---
fxc /Tps_3_0 /Zpr /nologo /Eps_main /Fcbuild/%shadername%.pasm %1

@echo.
@echo --- Assembling shader ---
psa /nologo /Fobuild/%shadername%.pso build/%shadername%.pasm

@echo.
@echo --- Copy shader to final directory ---
copy /y "build\%shadername%.pso" "../../../Bin/media/shaders"

@goto end_ok

:blank_err

@echo Please specify path of Pixel Shader HLSL to compile!
@echo example: build_pshader.bat C:\shadername.hlsl
@goto EXIT

:wrong_ext
@echo Provided file must have the .hlsl extension!
@goto EXIT

:end_ok

@echo.
@echo --- Compiling Ended ---

:EXIT
@REM should be last label