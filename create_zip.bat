@echo off
chcp 65001 >nul
echo ========================================
echo   创建发布 ZIP 包
echo ========================================
echo.

set RELEASE_DIR=release\ImageBatchTool
set ZIP_NAME=ImageBatchTool_v1.0.0_Release.zip

echo [1/2] 检查发布目录...
if not exist %RELEASE_DIR% (
    echo 错误：发布目录不存在，请先运行 package_release.bat
    pause
    exit /b 1
)

echo [2/2] 创建 ZIP 压缩包...
powershell -Command "Compress-Archive -Path '%RELEASE_DIR%\*' -DestinationPath 'release\%ZIP_NAME%' -Force"

if errorlevel 1 (
    echo 错误：压缩失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo   压缩完成！
echo ========================================
echo.
echo 文件位置：release\%ZIP_NAME%
echo.
echo 文件大小：
powershell -Command "(Get-Item 'release\%ZIP_NAME%').Length / 1MB | ForEach-Object { '{0:N2} MB' -f $_ }"
echo.
echo 发布清单：
echo ✓ ImageBatchTool.exe（主程序）
echo ✓ MSVC 运行时 DLL（3个）
echo ✓ 使用说明.txt
echo ✓ README.md
echo ✓ assets/ 目录
echo ✓ output/ 目录
echo.
echo 下一步：
echo 1. 在干净的 Windows 系统上测试
echo 2. 确认无需安装任何依赖即可运行
echo 3. 发布给用户
echo.
echo ========================================
pause

