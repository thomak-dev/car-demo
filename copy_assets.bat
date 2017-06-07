@ECHO OFF
ECHO %~nx0: Copying assets from %~dp0Assets to %~dp0%1
IF NOT "%~1"=="" (XCOPY %~dp0Assets %~1 /E /I /Y /Q /Exclude:%~dp0exclude_assets.txt) ELSE GOTO end
%~dp0"copy_libs_%~2.bat" %1
:end