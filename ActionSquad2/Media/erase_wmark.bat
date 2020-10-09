@cls
@IF NOT EXIST %~dp0texconv.exe GOTO NoExe

@SET workingDir=..\Bin\Data

@echo --- Processed Directory: %~dp0%workingDir% ---
for /r %workingDir% %%x IN (*.png) DO %~dp0texconv.exe -f A8R8G8B8 -o %%~dpx -ft PNG -nologo %%x
for /r %workingDir% %%x IN (*.jpg) DO %~dp0texconv.exe -f A8R8G8B8 -o %%~dpx -ft JPG -nologo %%x
@GOTO End

:NoParam
@echo Please provide a valid directory as the first parameter to this batch file.
@echo This script removes the watermark of all PNG and JPG files in all subfolders.
@GOTO End

:NoExe
@echo This script needs texconv.exe from the DX SDK December 2004 in the same folder!

:End
@echo --- Script Ended ---