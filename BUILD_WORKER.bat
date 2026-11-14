@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title DistriOpt - Build Worker Agent

echo ============================================
echo      BUILD DISTRIOPT WORKER AGENT
echo ============================================
echo.

where cmake >nul 2>nul
if not errorlevel 1 (
    echo [1/2] Configuring C++17 with CMake...
    cmake -S . -B build
    if not errorlevel 1 (
        echo [2/2] Building worker target...
        cmake --build build --config Release --target distriopt_worker
        if not errorlevel 1 (
            echo.
            echo SUCCESS: Worker built with CMake.
            goto done
        )
    )
    echo CMake build failed, falling back to G++...
)

where g++ >nul 2>nul
if not errorlevel 1 (
    echo Building worker agent directly with G++...
    g++ -std=c++17 -O2 -I include src/worker_agent.cpp -o distriopt_worker.exe -lws2_32
    if not errorlevel 1 (
        echo.
        echo SUCCESS: distriopt_worker.exe created successfully.
        goto done
    )
)

if exist "distriopt_worker.exe" (
    echo Precompiled distriopt_worker.exe is already available.
    goto done
)
if exist "build\Release\distriopt_worker.exe" (
    echo Precompiled worker is available at build\Release\distriopt_worker.exe.
    goto done
)

echo ERROR: Could not build worker agent. Ensure Visual Studio C++ or MinGW g++ is installed.
pause
exit /b 1

:done
echo You can run the worker using RUN_WORKER.bat
pause
exit /b 0
