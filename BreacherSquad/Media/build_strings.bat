@ECHO ---- Building Strings ----
@cd PreData
@cd Strings
@TextsCompiler.exe REWRITE strings_en_src.xml strings.xml
@copy strings.h ..\..\..\Source\Engine\Constants /y
@copy strings.xml ..\..\..\Bin\Media\texts /y

goto comment

@ECHO --- Building other languages ---
@ECHO -- Russian
@TextsCompiler.exe REWRITE localized/strings_ru_src.xml localized/strings_ru.xml
@ECHO -- German
@TextsCompiler.exe REWRITE localized/strings_de_src.xml localized/strings_de.xml
@ECHO -- French
@TextsCompiler.exe REWRITE localized/strings_fr_src.xml localized/strings_fr.xml
@ECHO -- Spanish
@TextsCompiler.exe REWRITE localized/strings_es_src.xml localized/strings_es.xml
@ECHO -- Chinese
@TextsCompiler.exe REWRITE localized/strings_cn_src.xml localized/strings_cn.xml
@ECHO -- Japanese
@TextsCompiler.exe REWRITE localized/strings_jp_src.xml localized/strings_jp.xml
@ECHO -- Korean
@TextsCompiler.exe REWRITE localized/strings_kr_src.xml localized/strings_kr.xml
@ECHO -- Portuguese
@TextsCompiler.exe REWRITE localized/strings_pt_src.xml localized/strings_pt.xml
@ECHO -- Portuguese-Brasilian
@TextsCompiler.exe REWRITE localized/strings_ptbr_src.xml localized/strings_ptbr.xml
@ECHO -- Italian
@TextsCompiler.exe REWRITE localized/strings_it_src.xml localized/strings_it.xml

:comment

@cd ..\..

@ECHO --- DONE BUILDING RESOURCES ---