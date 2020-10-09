@if [%1] == [] GOTO NoParams

:AllParams
@cd ..
ActionSquad.exe +upload_workshop %1
@cd tools

:NoParams
@echo ecample: mod_upload.bat "C:\temp\mod_root"
@echo You need to specify the path to the mod folder that contains the "mod_desc.xml" file (without the last slash or backslash). 
@echo Make sure you also have write access to the specified folder.

:FINISHED_OK
@echo Exiting...
