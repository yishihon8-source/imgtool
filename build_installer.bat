@echo off
chcp 65001 >nul
echo ========================================
echo   编译 Inno Setup 安装包
echo ========================================
echo.

set INNO_SETUP="D:\Inno Setup 6\ISCC.exe"
set SCRIPT_FILE=release\setup.iss.iss

echo [1/3] 检查 Inno Setup...
if not exist %INNO_SETUP% (
    echo 错误：找不到 Inno Setup 编译器
    echo 路径：%INNO_SETUP%
    pause
    exit /b 1
)
echo ✓ Inno Setup 已找到

echo [2/3] 检查脚本文件...
if not exist %SCRIPT_FILE% (
    echo 错误：找不到安装脚本
    echo 路径：%SCRIPT_FILE%
    pause
    exit /b 1
)
echo ✓ 脚本文件已找到

echo [3/3] 编译安装包...
cd release
%INNO_SETUP% setup.iss.iss

if errorlevel 1 (
    echo.
    echo ❌ 编译失败！
    pause
    exit /b 1
)

echo.
echo ========================================
echo   编译成功！
echo ========================================
echo.
echo 安装包位置：release\ImageBatchTool_Setup_v1.0.0.exe
echo.
echo 安装包大小：
cd ..
powershell -Command "if (Test-Path 'release\ImageBatchTool_Setup_v1.0.0.exe') { (Get-Item 'release\ImageBatchTool_Setup_v1.0.0.exe').Length / 1MB | ForEach-Object { '{0:N2} MB' -f $_ } } else { 'File not found' }"
echo.
echo 下一步：
echo 1. 测试安装包
echo 2. 在干净系统上安装测试
echo 3. 发布给用户
echo.
echo ========================================
pause

