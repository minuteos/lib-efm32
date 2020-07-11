@echo off
setlocal enableextensions enabledelayedexpansion
set ESC=
set RED=%ESC%[91m
set GREEN=%ESC%[92m
set BOLD=%ESC%[1m
set NORM=%ESC%[0m

if "%DEVICE%" == "" set DEVICE="bgm13p22"
if "%COMMANDER%" == "" set COMMANDER="c:\SiliconLabs\SimplicityStudio\v4\developer\adapter_packs\commander\commander.exe"
if "%IMAGE%" == "" for %%F in ("bin\*-signed.axf") do set IMAGE="%%~dpnF.s37"
if "%IMAGE%" == "" for %%F in ("bin\*.axf") do set IMAGE="%%~dpnF.s37"
set DEVICE_ARGS="-d %DEVICE% --speed=4000"

if not exist %IMAGE% (
    echo No image found for flashing
    exit /B -1
)

echo Using image: %IMAGE%

for %%T in ("app-encrypt-key*txt" "app-sign-key.pem-tokens*txt") do (
    echo Using manufacturing tokens from file: %%T
    set TOKEN_CONFIG=!TOKEN_CONFIG! --tokenfile %%T
)

if "%NO_ERASE%" == "" (
    echo %BOLD%NEW DEVICE MODE:%NORM% devices will be completely erased
) else (
    echo %BOLD%UPDATE MODE:%NORM% device data will not be erased, only firmware will be updated
)

:waitForDevice
    echo Waiting for device, press X to abort...

:waitForDevice2
    %COMMANDER% device %DEVICE_ARGS% info >NUL
    if errorlevel 0 (
        echo Device found.
        goto program
    )

    choice /C YX /T 1 /N /D Y >NUL
    if errorlevel 2 (
        echo %RED%ABORTED%NORM%
        exit /B -1
    )
    goto waitForDevice2

:program
    if "%NO_ERASE%" == "" (
        echo Mass erasing flash...
        %COMMANDER% device %DEVICE_ARGS% masserase
        if errorlevel 1 goto error
    )

    if not "%TOKEN_CONFIG%" == "" (
        echo Flashing manufacturing tokens...
        %COMMANDER% flash %DEVICE_ARGS% --tokengroup znet %TOKEN_CONFIG%
        if errorlevel 1 goto error
    )

    echo Flashing main image from %IMAGE%...
    %COMMANDER% flash %DEVICE_ARGS% %IMAGE%
    if errorlevel 1 goto error

    if "%NO_ERASE%" == "" (
        echo Locking debug access...
        %COMMANDER% device %DEVICE_ARGS% lock --debug enable
        if errorlevel 1 goto error
    )

    echo Resetting device...
    %COMMANDER% device reset
    if errorlevel 1 goto error

    echo.
    echo %GREEN%Success%NORM%
    echo.
    timeout /T 3 /NOBREAK >NUL
    goto waitForDevice

:error
    echo.
    echo %RED%!!! ERROR !!!%NORM%
    echo.
    timeout /T 5 /NOBREAK >NUL
    goto :waitForDevice
