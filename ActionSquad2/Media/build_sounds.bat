@ECHO ---- Extracting Sound Constants ----

@cd PreData\ExtractConstants
@copy ..\..\..\Bin\Media\Sounds\sounds.xml sounds.xml /y
@ExtractHfromXML.exe sounds.xml sounds.h Sound ID SNDIDX_
@del sounds.xml
@copy *.h ..\..\..\Source\Engine\Constants /y

@ECHO --- DONE ---