@echo off
setlocal
cd /d "%~dp0"
title DistriOpt - Core Unit Tests

echo ============================================
echo      DISTRIOPT CORE UNIT TESTS
echo ============================================
echo.

set "TESTS_EXE="
if exist "distriopt_tests.exe" set "TESTS_EXE=distriopt_tests.exe"
if exist "build\Release\distriopt_tests.exe" set "TESTS_EXE=build\Release\distriopt_tests.exe"
if exist "build\distriopt_tests.exe" set "TESTS_EXE=build\distriopt_tests.exe"

where cmake >nul 2>nul
if not errorlevel 1 (
    echo Building tests with CMake...
    cmake -S . -B build
    cmake --build build --config Release --target distriopt_tests
    if exist "build\Release\distriopt_tests.exe" set "TESTS_EXE=build\Release\distriopt_tests.exe"
    if exist "build\distriopt_tests.exe" set "TESTS_EXE=build\distriopt_tests.exe"
)

if not defined TESTS_EXE (
    where g++ >nul 2>nul
    if not errorlevel 1 (
        echo Building tests directly with G++...
        g++ -std=c++17 -O2 -I include tests/core_tests.cpp src/cluster.cpp src/scheduler.cpp src/storage.cpp -o distriopt_tests.exe -lws2_32
        if exist "distriopt_tests.exe" set "TESTS_EXE=distriopt_tests.exe"
    )
)

if not defined TESTS_EXE (
    echo ERROR: distriopt_tests.exe could not be built or found.
    pause
    exit /b 1
)

echo.
echo Running tests: %TESTS_EXE%
echo.
"%TESTS_EXE%"
echo.
pause
