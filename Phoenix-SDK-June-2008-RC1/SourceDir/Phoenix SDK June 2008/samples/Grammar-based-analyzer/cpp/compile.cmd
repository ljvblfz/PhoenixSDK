
:: Syntax: compile foo.min

:: Output: bin\foo.exe and foo-mtrace.exe

set bareName=%1
set bareName=%bareName:~0,-4%

nmake bin\frontend.exe ..\mtrace\csharp\bin\debug\mtrace.exe
bin\frontend.exe %1 > bin\%bareName%.ilasm
if errorlevel 1 exit /b 1
ilasm /debug bin\%bareName%.ilasm /output=bin\%bareName%.exe
..\mtrace\csharp\bin\debug\mtrace.exe bin\%bareName%.exe
