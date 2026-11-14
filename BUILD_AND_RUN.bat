@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title DistriOpt - Distributed Workload Optimization

echo ============================================
echo      DISTRIOPT - COMPUTE CONTROL PLANE
echo ============================================
echo.

set "SERVER_EXE="
if exist "distriopt_server.exe" set "SERVER_EXE=distriopt_server.exe"
if exist "build\Release\distriopt_server.exe" set "SERVER_EXE=build\Release\distriopt_server.exe"
if exist "build\distriopt_server.exe" set "SERVER_EXE=build\distriopt_server.exe"

where cmake >nul 2>nul
if not errorlevel 1 (
    echo [1/3] CMake detected. Configuring C++17 project...
    cmake -S . -B build
    if not errorlevel 1 (
        echo.
        echo [2/3] Building controller, worker agent and tests with CMake...
        cmake --build build --config Release
        if not errorlevel 1 (
            if exist "build\Release\distriopt_server.exe" set "SERVER_EXE=build\Release\distriopt_server.exe"
            if exist "build\distriopt_server.exe" set "SERVER_EXE=build\distriopt_server.exe"
            goto run_server
        )
    )
    echo CMake build failed, falling back to direct G++ compiler...
)

where g++ >nul 2>nul
if not errorlevel 1 (
    echo [1/2] G++ compiler detected. Building DistriOpt controller directly...
    g++ -std=c++17 -O2 -I include src/main.cpp src/http_server.cpp src/cluster.cpp src/scheduler.cpp src/storage.cpp -o distriopt_server.exe -lws2_32
    if not errorlevel 1 (
        echo [2/2] Building worker agent and core tests...
        g++ -std=c++17 -O2 -I include src/worker_agent.cpp -o distriopt_worker.exe -lws2_32
        g++ -std=c++17 -O2 -I include tests/core_tests.cpp src/cluster.cpp src/scheduler.cpp src/storage.cpp -o distriopt_tests.exe -lws2_32
        set "SERVER_EXE=distriopt_server.exe"
        goto run_server
    )
)

if defined SERVER_EXE (
    echo Using existing compiled controller: %SERVER_EXE%
    goto run_server
)

echo.
echo ERROR: Neither CMake nor G++ (MinGW/MSYS2) compiler was found in PATH,
echo and no prebuilt executable was found.
echo.
echo Please install either:
echo   1. Visual Studio 2022 with "Desktop development with C++" and CMake
echo   OR
echo   2. MinGW-w64 / MSYS2 with g++
echo.
pause
exit /b 1

:run_server
echo.
echo ============================================
echo   Starting DistriOpt Controller Server...
echo ============================================
echo.
echo Dashboard URL: http://localhost:8080
echo IMPORTANT: Keep this terminal window open while using DistriOpt.
echo Press Ctrl+C to stop the server.
echo.
start "" "http://localhost:8080"
"%SERVER_EXE%" 8080
pause
exit /b 0
