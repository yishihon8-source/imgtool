@echo off
echo ========================================
echo 使用 Visual Studio 构建 ImageBatchTool
echo ========================================
echo.

REM 查找 Visual Studio 安装路径
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo 错误: 未找到 Visual Studio
    echo 请安装 Visual Studio 2019 或 2022
    echo 下载地址: https://visualstudio.microsoft.com/zh-hans/downloads/
    pause
    exit /b 1
)

REM 获取 VS 安装路径
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSINSTALLDIR=%%i"
)

if not defined VSINSTALLDIR (
    echo 错误: 未找到 Visual Studio C++ 工具
    echo 请确保已安装 "使用 C++ 的桌面开发" 工作负载
    pause
    exit /b 1
)

echo 找到 Visual Studio: %VSINSTALLDIR%
echo.

REM 设置 VS 环境变量
call "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvars64.bat"

REM 刷新 PATH 以包含 CMake
set "PATH=%PATH%;C:\Program Files\CMake\bin"

REM 检查依赖库
if not exist "third_party\imgui" (
    echo 错误: 未找到 ImGui，请先运行 download_dependencies.bat
    pause
    exit /b 1
)

REM 创建构建目录
if not exist "build" mkdir build
cd build

echo.
echo [1/2] 配置项目...
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo 错误: CMake 配置失败
    cd ..
    pause
    exit /b 1
)

echo.
echo [2/2] 编译项目...
nmake
if %ERRORLEVEL% NEQ 0 (
    echo 错误: 编译失败
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo 构建成功！
echo ========================================
echo.
echo 可执行文件: build\ImageBatchTool.exe
echo.
pause
