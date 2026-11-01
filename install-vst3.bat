@echo off
setlocal
title ArcWave Installer

set "SRC=%~dp0dist\ArcWave.vst3"
if not exist "%SRC%" set "SRC=%~dp0ArcWave.vst3"
if not exist "%SRC%" (
    echo.
    echo Could not find ArcWave.vst3 next to this script.
    echo Expected either "dist\ArcWave.vst3" or "ArcWave.vst3".
    echo.
    pause
    exit /b 1
)

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Requesting administrator privileges...
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

set "DEST=C:\Program Files\Common Files\VST3\ArcWave.vst3"
if exist "%DEST%" rmdir /s /q "%DEST%"
xcopy /E /I /Y "%SRC%" "%DEST%" >nul
if errorlevel 1 (
    echo.
    echo Install failed. Copy the ArcWave.vst3 folder manually to:
    echo   %DEST%
    echo.
    pause
    exit /b 1
)

echo.
echo ArcWave installed to:
echo   %DEST%
echo.
echo In FL Studio: Options ^> Manage plugins ^> Find plugins, then rescan.
echo.
pause
