@echo off
setlocal
cd /d "%~dp0"
title DistriOpt - Live Worker Agent

echo ============================================
echo      DISTRIOPT LIVE WORKER AGENT
echo ============================================
echo.

set "WORKER_EXE="
if exist "distriopt_worker.exe" set "WORKER_EXE=distriopt_worker.exe"
if exist "build\Release\distriopt_worker.exe" set "WORKER_EXE=build\Release\distriopt_worker.exe"
if exist "build\distriopt_worker.exe" set "WORKER_EXE=build\distriopt_worker.exe"

if not defined WORKER_EXE (
    echo Worker executable not found. Running BUILD_WORKER.bat...
    call BUILD_WORKER.bat
    if exist "distriopt_worker.exe" set "WORKER_EXE=distriopt_worker.exe"
)

if not defined WORKER_EXE (
    echo ERROR: distriopt_worker.exe could not be found or built.
    pause
    exit /b 1
)

set /p CONTROLLER_IP=Enter controller laptop IPv4 address [Default 127.0.0.1]: 
if "%CONTROLLER_IP%"=="" set CONTROLLER_IP=127.0.0.1

set /p WORKER_NAME=Enter a unique worker name [Default %COMPUTERNAME%]: 
if "%WORKER_NAME%"=="" set WORKER_NAME=%COMPUTERNAME%

echo.
echo Starting worker '%WORKER_NAME%' connecting to %CONTROLLER_IP%:8080 ...
echo.
"%WORKER_EXE%" %CONTROLLER_IP% 8080 "%WORKER_NAME%"
pause
