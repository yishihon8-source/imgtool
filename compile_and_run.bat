@echo off
chcp 65001 >nul
title Image Batch Tool - 编译和运行

echo.
echo ============================================================================
echo   Image Batch Tool - 编译运行脚本
echo   生成日期: 2026-01-31
echo ============================================================================
echo.

setlocal enabledelayedexpansion

REM 检查 CMake
echo [1/5] 检查 CMake...
where cmake >nul 2>&1
if errorlevel 1 (
    echo 错误: 未找到 CMake
    echo 请下载并安装 CMake: https://cmake.org/download/
    pause
    exit /b 1
)
echo ✓ CMake 已安装

REM 清理旧的构建
echo.
echo [2/5] 清理旧的构建目录...
if exist build_release (
    echo 正在删除 build_release...
    rmdir /s /q build_release >nul 2>&1
)
echo ✓ 清理完成

REM 创建构建目录
echo.
echo [3/5] 创建构建目录...
mkdir build_release
cd build_release
echo ✓ 目录已创建

REM CMake 配置
echo.
echo [4/5] 配置 CMake...
echo.
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release .. 2>&1 | tee cmake_config.log
if errorlevel 1 (
    echo.
    echo 警告: CMake 配置可能出现问题
    echo 如果您使用的是不同版本的 Visual Studio，请修改 -G 参数:
    echo   VS 2019: -G "Visual Studio 16 2019"
    echo   VS 2022: -G "Visual Studio 17 2022"
    echo.
    echo 也可以尝试使用 Unix Makefiles (需要 MinGW):
    echo   cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    echo.
    pause
    exit /b 1
)

REM 编译
echo.
echo [5/5] 编译项目...
echo.
cmake --build . --config Release --parallel 4 2>&1 | tee cmake_build.log
if errorlevel 1 (
    echo.
    echo 错误: 编译失败
    echo 请检查上面的错误信息
    echo.
    pause
    exit /b 1
)

echo.
echo ============================================================================
echo   ✓ 编译成功！
echo ============================================================================
echo.

REM 检查可执行文件
if exist "bin\Release\ImageBatchTool.exe" (
    echo ✓ 可执行文件: bin\Release\ImageBatchTool.exe
    echo.
    echo 准备启动应用程序...
    echo.
    pause
    
    REM 运行程序
    cd ..
    start build_release\bin\Release\ImageBatchTool.exe
    
    echo.
    echo ✓ 程序已启动
    echo.
) else (
    echo.
    echo 警告: 未找到可执行文件
    echo 期望位置: bin\Release\ImageBatchTool.exe
    echo.
    pause
)

endlocal
