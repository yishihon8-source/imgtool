@echo off
setlocal enabledelayedexpansion

echo =====================================
echo Git Auto Installer
echo =====================================
echo.

REM 下载 Git
set "URL=https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe"
set "INSTALLER=%TEMP%\git-installer.exe"

echo Step 1: Downloading Git...
echo Downloading from GitHub...

powershell -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; try { Invoke-WebRequest -Uri '%URL%' -OutFile '%INSTALLER%' -ErrorAction Stop; Write-Host 'Download Complete'; exit 0 } catch { Write-Host 'Download Failed'; exit 1 }"

if errorlevel 1 (
    echo Failed to download Git
    pause
    exit /b 1
)

echo.
echo Step 2: Installing Git...
echo Please wait 30-60 seconds...
echo.

REM Use different installation method
"%INSTALLER%" /VERYSILENT /NORESTART

echo Installation completed
echo Step 3: Verifying installation...
timeout /t 2 /nobreak

if exist "C:\Program Files\Git\bin\git.exe" (
    echo.
    echo SUCCESS: Git installed successfully!
    echo.
    "C:\Program Files\Git\bin\git.exe" --version
    
    REM Delete installer
    del /f /q "%INSTALLER%"
    
    echo.
    echo Next step: Run the upload script
    echo cd f:\oneDrive\Desktop\ps-cpp
    echo auto_push_to_github.bat
) else (
    echo.
    echo WARNING: Git installation may not be complete
    echo Please restart PowerShell and try again
)

echo.
pause
