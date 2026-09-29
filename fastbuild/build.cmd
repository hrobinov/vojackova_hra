@echo off
rem Builds the game: finds MSVC and the Windows SDK, then runs FASTBuild with
rem whatever arguments it was given, from any folder:
rem   fastbuild\build.cmd Debug
rem   fastbuild\build.cmd Release -clean
rem
rem It hands them over in the variables a Visual Studio developer prompt sets
rem (VCToolsInstallDir, WindowsSdkDir, WindowsSDKVersion), and from one of
rem those it leaves them as they are -- so from there FBuild.exe works directly.
rem
rem No if ( ... ) blocks here: %ProgramFiles(x86)% expands to a path with a
rem ")" in it, which would end the block early. Hence the gotos.
setlocal
cd /d "%~dp0.."

if defined VCToolsInstallDir if defined WindowsSdkDir if defined WindowsSDKVersion goto build

rem MSVC: the newest Visual Studio or Build Tools with the C++ tools, and the
rem toolset version it defaults to.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto novs
set "VSINSTALL="
for /f "delims=" %%i in ('call "%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set "VSINSTALL=%%i"
if not defined VSINSTALL goto novs
set "VCTOOLSVERSION="
set /p VCTOOLSVERSION=<"%VSINSTALL%\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt"
if not defined VCTOOLSVERSION goto novs
set "VCToolsInstallDir=%VSINSTALL%\VC\Tools\MSVC\%VCTOOLSVERSION%\"

rem The Windows SDK: the newest one with the desktop headers. Some version
rem folders hold only the UCRT, so each is checked for windows.h.
set "WindowsSdkDir=%ProgramFiles(x86)%\Windows Kits\10\"
set "WindowsSDKVersion="
for /f "delims=" %%v in ('dir /b /ad /on "%WindowsSdkDir%Include\10.*" 2^>nul') do if exist "%WindowsSdkDir%Include\%%v\um\windows.h" set "WindowsSDKVersion=%%v\"
if not defined WindowsSDKVersion goto nosdk

:build
"%~dp0FBuild.exe" -config "%~dp0fbuild.bff" %*
exit /b %errorlevel%

:novs
echo build.cmd: no Visual Studio or Build Tools with the C++ tools found. 1>&2
exit /b 1

:nosdk
echo build.cmd: no Windows 10/11 SDK found under %WindowsSdkDir% 1>&2
exit /b 1
