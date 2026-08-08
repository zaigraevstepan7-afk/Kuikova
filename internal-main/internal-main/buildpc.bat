@echo off
setlocal enableextensions

set CMAKE=C:\Program Files\CMake\bin\cmake.exe
set "SRC=%~dp0"
if "%SRC:~-1%"=="\" set "SRC=%SRC:~0,-1%"

pushd "%SRC%"

echo Available configure presets:
"%CMAKE%" --list-presets
echo.

set PRESETS=release-pc

for %%P in (%PRESETS%) do (
    echo.
    echo Configuring preset: %%P
    echo ----------------------

    "%CMAKE%" --preset %%P
    if errorlevel 1 goto fail

    echo.
    echo Building preset: %%P
    echo --------------------

    "%CMAKE%" --build "%SRC%\build\%%P" --parallel 7
    if errorlevel 1 goto fail
)

popd

echo.
echo ##############  BUILD SUCCESS  ##############
pause
exit /b 0

:fail
popd
echo.
echo ##############  BUILD FAILED (%errorlevel%)  ##############
pause
exit /b %errorlevel%
