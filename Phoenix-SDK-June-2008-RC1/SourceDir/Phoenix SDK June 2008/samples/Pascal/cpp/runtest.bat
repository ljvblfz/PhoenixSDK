@ECHO OFF
IF "%1"=="" GOTO error
DEL /Q %1\Tests\*.*
IF ERRORLEVEL 1 GOTO makedir
GOTO clean
:makedir
MKDIR %1\Tests
GOTO clean
:clean
ECHO Copying source files...
XCOPY /E /Y Test\Tests\*.* %1\Tests
ECHO Copying XML Schema...
COPY Test\Schema\Test.xsd %1\Tests
ECHO Copying mspvcrt.lib...
COPY %1\mspvcrt.lib %1\Tests
ECHO Copying msp.exe...
COPY %1\msp.exe %1\Tests
ECHO Copying mspt.exe...
COPY %1\mspt.exe %1\Tests
CD %1\Tests
mspt.exe -print -visits -verify
CD..\..
GOTO exit
:error
ECHO "Format: runtest.bat {debug|release}"
:exit