@echo off
rem ---- locate the Visual Studio x86 C++ build tools ------------------------
rem Skipped when this already runs inside a developer prompt.
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
  echo Install "Visual Studio Build Tools" with the "Desktop development with C++"
  echo workload, or run this script from an "x86 Native Tools Command Prompt".
  exit /b 1
)
:vsenv_call
call "%VCVARS%" >nul
:vsenv_done
rem ---- all paths derive from this script's location -------------------------
set "PV=%~dp0"
if "%PV:~-1%"=="\" set "PV=%PV:~0,-1%"
for %%i in ("%PV%\..\..") do set "ENG=%%~fi"
set SMOKE=%ENG%\build\msvc-smoke
set OBJ=%PV%\obj-client
if not exist "%OBJ%" mkdir "%OBJ%"
del /q "%OBJ%\*.obj" 2>nul
set DEFS=/DWIN32 /D_X86_ /DGL=4 /DNET_ENET_2 /DBSP /DLUA_USE_APICHECK /DTEXTURE_PNG /DSOUND_SUPPORT /DSOUND_OPENAL ^
 /DDEBUG_ON /DDEBUG_COMP /DDEBUG_SPOTFX_SOUND /DDEBUG_VIEWPORT ^
 /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /D_WINSOCK_DEPRECATED_NO_WARNINGS
rem SDL2 headers replace the SDL1.2 compat\SDL dir; keep compat (unistd/GL/png shims)
set INCS=/I"%PV%\deps\include\SDL3" /I"%PV%\deps\include" /I"%SMOKE%\compat" /I"%ENG%" /I"%ENG%\gl2_loader" /I"%PV%\deps\include"
cd /d "%ENG%"
echo === compiling 100 originals (GL=4 native/DSA, SDL3) ===
cl /c /nologo /MD /std:c11 /W1 %DEFS% %INCS% /Fo"%OBJ%\\" @"%PV%\obj\originals.rsp" >"%PV%\cc-orig.log" 2>&1
if errorlevel 1 (echo ORIG_FAILED & exit /b 1)
echo === compiling 5 patched + gl2 loader ===
cl /c /nologo /MD /std:c11 /W1 %DEFS% %INCS% /Fo"%OBJ%\\" "%PV%\patched\stats.c" "%PV%\patched\xmem.c" "%PV%\patched\main.c" "%PV%\patched\restart.c" "%PV%\patched\title.c" "%ENG%\gl2_loader\gl2_load.c" >"%PV%\cc-patched.log" 2>&1
if errorlevel 1 (echo PATCHED_FAILED & exit /b 1)
echo COMPILE_OK
