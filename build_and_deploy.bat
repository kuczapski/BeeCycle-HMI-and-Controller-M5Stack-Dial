@echo off
setlocal

set "PROJECT_DIR=%~dp0"
set "MODE=%~1"
set "PORT=%~2"

if "%MODE%"=="" set "MODE=upload"
if "%PORT%"=="" set "PORT=COM5"

where py >nul 2>nul
if errorlevel 1 (
    echo Python launcher ^("py"^) was not found on PATH.
    exit /b 1
)

pushd "%PROJECT_DIR%" >nul
if errorlevel 1 (
    echo Failed to enter project directory: %PROJECT_DIR%
    exit /b 1
)

if /I "%MODE%"=="build" (
    echo Building firmware...
    py -m platformio run
    set "RESULT=%ERRORLEVEL%"
    popd >nul
    exit /b %RESULT%
)

if /I "%MODE%"=="upload" (
    echo Building and uploading firmware to %PORT%...
    py -m platformio run -t upload --upload-port %PORT%
    set "RESULT=%ERRORLEVEL%"
    popd >nul
    exit /b %RESULT%
)

echo Usage:
echo   %~nx0 build
echo   %~nx0 upload [COM_PORT]
popd >nul
exit /b 1