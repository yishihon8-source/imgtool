@echo off
chcp 65001 >nul
echo ========================================
echo   生成独立 EXE（无需任何依赖）
echo ========================================
echo.

REM 清理旧的构建
if exist build_release (
    echo [1/4] 清理旧构建...
    rmdir /s /q build_release
)

REM 创建构建目录
echo [2/4] 创建构建目录...
mkdir build_release
cd build_release

REM 配置 CMake（Release 模式，静态链接）
echo [3/4] 配置 CMake...
cmake -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release ..
if errorlevel 1 (
    echo.
    echo [错误] CMake 配置失败！
    echo 请确保已安装 Visual Studio 或 CMake
    pause
    exit /b 1
)

REM 编译项目
echo [4/4] 编译项目（Release 模式）...
cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo [错误] 编译失败！
    pause
    exit /b 1
)

echo.
echo ========================================
echo   ✓ 编译成功！
echo ========================================
echo.
echo 独立 EXE 位置：
echo   %cd%\bin\Release\ImageBatchTool.exe
echo.
echo 这个 exe 可以直接复制到任何 Windows 电脑运行！
echo 无需安装任何依赖（Visual C++、.NET 等）
echo.

REM 打开输出目录
explorer bin\Release

pause

