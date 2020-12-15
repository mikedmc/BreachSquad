@ECHO ---- Extracting Constants ----
@cd PreData
@ECHO ---- Building Strings ----
@cd Strings
@TextsCompiler.exe LOADALPHA alphabet.xml REWRITE strings_en.xml strings.xml
@copy strings.h ..\..\..\Source\Engine\Constants /y
@copy strings.xml ..\..\..\Bin\Media /y
@cd ..\..\..

@cd Media
@cd PreData
@SeppiaFilePacker BUILD Sounds/files/sounds.flist
@copy Sounds\sounds.xml ..\..\Bin\Media\sounds /y
@copy Sounds\files\sounds.sfp ..\..\Bin\Media\sounds /y

@cd ExtractConstants
@copy ..\Sounds\sounds.xml sounds.xml /y
@ExtractHfromXML.exe sounds.xml sounds.h Sound ID SNDIDX_
@del sounds.xml
@copy *.h ..\..\..\Source\Engine\Constants /y

@ECHO --- DONE BUILDING RESOURCES ---