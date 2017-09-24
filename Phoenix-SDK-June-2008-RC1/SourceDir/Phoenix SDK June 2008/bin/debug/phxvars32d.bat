@echo off
@echo Setting paths to the Phoenix Debug Environment
@SET VSINSTALLDIR=[VSINSTALLDIR]
@SET VCINSTALLDIR=[VCINSTALLDIR]
@SET FrameworkDir=[DOTNETINSTALLDIR]
@SET FrameworkVersion=[FRAMEWORKVERSION]
@SET Framework35Version=v3.5
@SET RDKRoot=[INSTALLDIR]
@SET PhoenixSdkUserRoot=[SAMPLESINSTALLDIR]
@if "%VSINSTALLDIR%"=="" goto error_no_VSINSTALLDIR

@rem 
@rem Root of the VS SDK install, if any.
@rem 
@setlocal
@if "%TMP%"=="" set TMP=%TEMP%
@if "%TMP%"=="" set TMP=.
@cscript //Nologo "%RDKRoot%\bin\common\setVsSdkInstallDir.js" > "%TMP%\setVsSdkInstallDir.bat"
@endlocal
@if %ERRORLEVEL% EQU 0 call "%TMP%\setVsSdkInstallDir.bat"
@del "%TMP%\setVsSdkInstallDir.bat"

rem If VC is installed, call vcvars32 for the appropriate toolset

:setVCVars

@if NOT EXIST "%VCINSTALLDIR%\vcvarsall.bat" (
   echo Warning! Visual C++ not installed or detected, C++ compilation will not be enabled.
   echo If Visual C++ is installed, please reinstall or repair the Phoenix SDK to enable.
   goto setBasicVars
)

@call "%VCINSTALLDIR%\vcvarsall.bat" %1

Rem if we set the vc vars up, there isn't any reason to set up the additional framework paths.

goto setPHXVars

:setBasicVars

Rem If we got here, VC isn't installed.  So, we should but the basic set of tools (framework) on the path

set PATH=%VSINSTALLDIR%\Common7\IDE\;%FrameworkDir%\%Framework35Version%;%FrameworkDir%\%FrameworkVersion%;%PATH%
set LIBPATH=%FrameworkDir%\%Framework35Version%;%FrameworkDir%\%FrameworkVersion%;%LIBPATH%

:setPHXVars

Rem set specific phoenix vars based on the arch
@if "%1"=="x86" set PATH=%RDKRoot%\bin\debug\x86\;%PATH%
@if "%1"=="x86_amd64" set PATH=%RDKRoot%\bin\debug\x86_amd64\;%RDKRoot%\bin\debug\x86\;%PATH%

Rem Launch the appropriate VS if desired

if NOT "%1"=="launchVS" goto noVS

devenv.exe /useenv

:noVS

if NOT "%1"=="launchVCE" goto noVCE

VCExpress.exe /useenv

:noVCE

@goto end

:error_no_VSINSTALLDIR
@echo ERROR: VSINSTALLDIR variable is not set. 
@goto end

:end