@echo off
setlocal

REM =========================================================
REM Windows + MSVC build script
REM Default build type: Debug (change to Release if needed)
REM =========================================================
set "BUILD_TYPE=Debug"

REM Resolve project root from scripts/ directory
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_win_msvc"

echo [INFO] Project root: %ROOT_DIR%
echo [INFO] Build dir   : %BUILD_DIR%
echo [INFO] Build type  : %BUILD_TYPE%

if not exist "%BUILD_DIR%" (
    echo [INFO] Creating build directory...
    mkdir "%BUILD_DIR%"
)

cd /d "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] Failed to enter build directory.
    exit /b 1
)

echo [INFO] Configuring with CMake (Visual Studio 17 2022, x64)...
cmake -G "Visual Studio 17 2022" -A x64 "%ROOT_DIR%"
if errorlevel 1 (
    echo [ERROR] CMake configure failed.
    exit /b 1
)

echo [INFO] Building...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo [INFO] Build completed successfully.
exit /b 0
