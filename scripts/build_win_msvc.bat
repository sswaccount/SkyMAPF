@echo off
setlocal enabledelayedexpansion

REM =========================================================
REM Windows + MSVC build script
REM Default: core + cli only
REM Optional components: test / bind / example
REM =========================================================

set "BUILD_TYPE=Debug"

REM You can override externally:
REM   set BUILD_TYPE=Release
REM   scripts\build_win_msvc.bat test bind
if not "%BUILD_TYPE%"=="" set "BUILD_TYPE=%BUILD_TYPE%"

REM Resolve project root from scripts/ directory
for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\build_win_msvc"

REM Default switches
set "BUILD_CLI=ON"
set "BUILD_TESTS=OFF"
set "BUILD_EXAMPLES=OFF"
set "BUILD_BINDINGS=OFF"
set "CLEAN_CONFIG=OFF"

:parse_args
if "%~1"=="" goto args_done

if /I "%~1"=="test" (
    set "BUILD_TESTS=ON"
    shift
    goto parse_args
)

if /I "%~1"=="example" (
    set "BUILD_EXAMPLES=ON"
    shift
    goto parse_args
)

if /I "%~1"=="bind" (
    set "BUILD_BINDINGS=ON"
    shift
    goto parse_args
)

if /I "%~1"=="cli" (
    set "BUILD_CLI=ON"
    shift
    goto parse_args
)

if /I "%~1"=="clean" (
    set "CLEAN_CONFIG=ON"
    shift
    goto parse_args
)

if /I "%~1"=="-h" goto usage
if /I "%~1"=="--help" goto usage
if /I "%~1"=="help" goto usage

echo [ERROR] Unknown argument: %~1
goto usage

:args_done

echo [INFO] Project root   : %ROOT_DIR%
echo [INFO] Build dir      : %BUILD_DIR%
echo [INFO] Build type     : %BUILD_TYPE%
echo [INFO] Build CLI      : %BUILD_CLI%
echo [INFO] Build tests    : %BUILD_TESTS%
echo [INFO] Build examples : %BUILD_EXAMPLES%
echo [INFO] Build bindings : %BUILD_BINDINGS%

if not exist "%BUILD_DIR%" (
    echo [INFO] Creating build directory...
    mkdir "%BUILD_DIR%"
)

if /I "%CLEAN_CONFIG%"=="ON" (
    echo [INFO] Cleaning CMake cache only...
    if exist "%BUILD_DIR%\CMakeCache.txt" del /f /q "%BUILD_DIR%\CMakeCache.txt"
    if exist "%BUILD_DIR%\CMakeFiles" rmdir /s /q "%BUILD_DIR%\CMakeFiles"
)

cd /d "%BUILD_DIR%"
if errorlevel 1 (
    echo [ERROR] Failed to enter build directory.
    exit /b 1
)

echo [INFO] Configuring with CMake (Visual Studio 17 2022, x64)...
cmake -G "Visual Studio 17 2022" -A x64 "%ROOT_DIR%" ^
  -DSKYMAPF_BUILD_CLI=%BUILD_CLI% ^
  -DSKYMAPF_BUILD_TESTS=%BUILD_TESTS% ^
  -DSKYMAPF_BUILD_EXAMPLES=%BUILD_EXAMPLES% ^
  -DSKYMAPF_BUILD_PYTHON_BINDINGS=%BUILD_BINDINGS%
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

:usage
echo.
echo Usage:
echo   build_win_msvc.bat [options] [components]
echo.
echo Components:
echo   test       Enable tests
echo   example    Enable examples
echo   bind       Enable python bindings
echo   cli        Enable cli
echo.
echo Options:
echo   clean      Remove CMakeCache.txt and CMakeFiles before configure
echo   help       Show this help
echo.
echo Examples:
echo   scripts\build_win_msvc.bat
echo   scripts\build_win_msvc.bat test
echo   scripts\build_win_msvc.bat bind example
echo   set BUILD_TYPE=Release ^&^& scripts\build_win_msvc.bat test bind
echo.
exit /b 1