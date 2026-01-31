@echo off
chcp 65001 >nul
echo ========================================
echo   打包独立 EXE 程序
echo ========================================
echo.

set EXE_PATH=build_release\bin\Release\ImageBatchTool.exe
set OUTPUT_DIR=发布版本

if not exist "%EXE_PATH%" (
    echo [错误] 找不到 exe 文件！
    echo 请先运行 build_release.bat 编译项目
    pause
    exit /b 1
)

echo [1/3] 创建发布目录...
if exist "%OUTPUT_DIR%" (
    rmdir /s /q "%OUTPUT_DIR%"
)
mkdir "%OUTPUT_DIR%"

echo [2/3] 复制 exe 文件...
copy "%EXE_PATH%" "%OUTPUT_DIR%\图片批处理工具.exe"

echo [3/3] 创建使用说明...
(
echo ========================================
echo   图片批处理工具 v1.0
echo ========================================
echo.
echo 【功能特点】
echo   ✓ 批量处理图片（调整大小、裁剪、对齐）
echo   ✓ 支持 JPG/PNG/BMP/TGA 格式
echo   ✓ 可视化画布系统
echo   ✓ 高质量输出（JPG 95质量 / PNG 无损）
echo.
echo 【使用方法】
echo   1. 双击运行"图片批处理工具.exe"
echo   2. 点击"添加文件夹"或"添加图片"导入素材
echo   3. 在右侧设置画布尺寸，点击"应用画布"
echo   4. 点击"开始处理"选择输出目录
echo.
echo 【系统要求】
echo   ✓ Windows 10/11（64位）
echo   ✓ 无需安装任何依赖
echo   ✓ 绿色软件，解压即用
echo.
echo 【快捷键】
echo   Ctrl+T  - 变换工具（移动/缩放图片）
echo   方向键  - 微调图片位置（1px）
echo   Shift+方向键 - 快速移动（10px）
echo   鼠标滚轮 - 画布上下滚动
echo.
echo ========================================
echo   技术支持：https://github.com/yourname
echo ========================================
) > "%OUTPUT_DIR%\使用说明.txt"

echo.
echo ========================================
echo   ✓ 打包完成！
echo ========================================
echo.
echo 发布文件位置：
echo   %cd%\%OUTPUT_DIR%
echo.
echo 文件列表：
dir /b "%OUTPUT_DIR%"
echo.
echo 可以直接把整个文件夹发给别人使用！
echo.

explorer "%OUTPUT_DIR%"
pause



