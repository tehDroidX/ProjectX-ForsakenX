@echo off
rem ---- locate the Visual Studio x86 C++ build tools ------------------------
if defined VSCMD_ARG_TGT_ARCH goto :vsenv_done
set "VCVARS="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :vswhere_skip
for /f "usebackq tokens=* delims=" %%v in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Auxiliary\Build\vcvars32.bat`) do set "VCVARS=%%v"
:vswhere_skip
if defined VCVARS goto :vsenv_call
for %%e in (2026 2022 2019 18 17) do (
  for %%d in (BuildTools Community Professional Enterprise) do (
    if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\%%e\%%d\VC\Auxiliary\Build\vcvars32.bat" set "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\%%e\%%d\VC\Auxiliary\Build\vcvars32.bat"
    if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\%%e\%%d\VC\Auxiliary\Build\vcvars32.bat" set "VCVARS=%ProgramFiles%\Microsoft Visual Studio\%%e\%%d\VC\Auxiliary\Build\vcvars32.bat"
  )
)
if not defined VCVARS (
  echo Could not find the Visual Studio C++ build tools ^(vcvars32.bat^).
  exit /b 1
)
:vsenv_call
call "%VCVARS%" >nul
:vsenv_done
set "PV=%~dp0"
if "%PV:~-1%"=="\" set "PV=%PV:~0,-1%"
set D=%PV%\deps\lib
cd /d "%PV%"
link /NOLOGO /OUT:"%PV%\projectx_client_gl1t.exe" /SUBSYSTEM:WINDOWS /MACHINE:X86 obj-client\*.obj "%D%\SDLmain.lib" "%D%\SDL.lib" "%D%\lua.lib" "%D%\luasocket.lib" "%D%\enet.lib" "%D%\libpng.lib" "%D%\zlib.lib" "%D%\OpenAL32.lib" opengl32.lib glu32.lib ws2_32.lib winmm.lib user32.lib gdi32.lib advapi32.lib shell32.lib ole32.lib version.lib legacy_stdio_definitions.lib >"%PV%\link-client.log" 2>&1
echo LINK_EXIT=%ERRORLEVEL%
