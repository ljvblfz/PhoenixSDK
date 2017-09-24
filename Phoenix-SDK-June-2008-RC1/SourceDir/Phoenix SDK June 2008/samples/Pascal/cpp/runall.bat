@ECHO OFF
MSBuild.exe Pascal.sln /t:Rebuild /p:Configuration=Debug
CALL runtest.bat Debug
MSBuild.exe Pascal.sln /t:Rebuild /p:Configuration=Release
CALL runtest.bat Release