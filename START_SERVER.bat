@echo off
setlocal
cd /d "%~dp0"
title DistriOpt - Controller Server

echo ============================================
echo   STARTING DISTRIOPT CONTROLLER SERVER
echo ============================================
echo.

set "SERVER_EXE="
if exist "distriopt_server.exe" set "SERVER_EXE=distriopt_server.exe"
if exist "build\Release\distriopt_server.exe" set "SERVER_EXE=build\Release\distriopt_server.exe"
if exist "build\distriopt_server.exe" set "SERVER_EXE=build\distriopt_server.exe"

if not defined SERVER_EXE (
    echo Executable not found. Running BUILD_AND_RUN.bat...
    call BUILD_AND_RUN.bat
    exit /b %errorlevel%
)

echo Opening dashboard in browser: http://localhost:8080
start "" "http://localhost:8080"
echo Server running on port 8080. Press Ctrl+C to stop.
echo.
"%SERVER_EXE%" 8080
pause
