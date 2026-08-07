@echo off
setlocal EnableExtensions EnableDelayedExpansion
set "ADB=C:\Users\lkwelf3f3\Desktop\platform-tools\adb.exe"
set "liba=%~dp0"
set "DEV_DIR=/data/local/tmp/w1"
set "PKG=com.axlebolt.standoff2"
"%ADB%" get-state >nul 2>&1 || (echo no device & pause & exit /b 1)
"%ADB%" push "%liba%libs\arm64-v8a\libpayload.so" "%DEV_DIR%/libpayload.so" >nul
"%ADB%" push "%liba%libs\arm64-v8a\inj" "%DEV_DIR%/inj" >nul
"%ADB%" shell "chmod 755 %DEV_DIR%/inj"
set "PID="
for /l %%i in (1,1,30) do (
    for /f "delims=" %%p in ('"%ADB%" shell pidof %PKG% 2^>nul') do set "PID=%%p"
    if defined PID for /f "delims= " %%a in ("!PID!") do set "PID=%%a"
    if defined PID goto pid_ok
    timeout /t 2 /nobreak >nul
)
if not defined PID set /p "PID=enter game pid: "
:pid_ok
"%ADB%" logcat -c
"%ADB%" shell "su -c 'cd %DEV_DIR% && ./inj !PID! ./libpayload.so'"
pause
