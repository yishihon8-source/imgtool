@echo off
chcp 65001 >nul
echo ========================================
echo   ImageBatchTool 商业级发布打包工具
echo ========================================
echo.

set BUILD_DIR=build\bin\Release
set RELEASE_DIR=release\ImageBatchTool

echo [1/5] 清理旧的发布目录...
if exist release rmdir /s /q release
mkdir %RELEASE_DIR%

echo [2/5] 复制主程序...
copy %BUILD_DIR%\ImageBatchTool.exe %RELEASE_DIR%\
if errorlevel 1 (
    echo 错误：找不到 ImageBatchTool.exe，请先编译项目！
    pause
    exit /b 1
)

echo [3/5] 复制 MSVC 运行时 DLL（动态链接）...
REM 从 Visual Studio 安装目录复制运行时
set VS_REDIST=D:\vsstudio\VC\Redist\MSVC\14.50.35717\x64\Microsoft.VC143.CRT

if exist "%VS_REDIST%\msvcp140.dll" (
    copy "%VS_REDIST%\msvcp140.dll" %RELEASE_DIR%\
    copy "%VS_REDIST%\vcruntime140.dll" %RELEASE_DIR%\
    copy "%VS_REDIST%\vcruntime140_1.dll" %RELEASE_DIR%\
    echo ✓ 从 VS 目录复制运行时成功
) else (
    REM 备用方案：从系统目录复制
    echo 从系统目录复制运行时...
    copy C:\Windows\System32\msvcp140.dll %RELEASE_DIR%\
    copy C:\Windows\System32\vcruntime140.dll %RELEASE_DIR%\
    copy C:\Windows\System32\vcruntime140_1.dll %RELEASE_DIR%\
    echo ✓ 从系统目录复制运行时成功
)

echo [4/5] 创建发布目录结构...
mkdir %RELEASE_DIR%\assets
mkdir %RELEASE_DIR%\output

echo [5/5] 复制使用说明...
if exist "发布版本\使用说明.txt" (
    copy "发布版本\使用说明.txt" %RELEASE_DIR%\
)

echo.
echo ========================================
echo   打包完成！
echo ========================================
echo.
echo 发布目录：%RELEASE_DIR%
echo.
echo 目录结构：
dir /b %RELEASE_DIR%
echo.
echo 下一步：
echo 1. 测试运行 ImageBatchTool.exe
echo 2. 压缩 release\ImageBatchTool 文件夹为 ZIP
echo 3. 发布给用户
echo.
echo ========================================
pause

