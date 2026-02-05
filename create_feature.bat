@echo off
chcp 65001 >nul
echo ========================================
echo   创建新功能分支 (Feature Branch)
echo ========================================
echo.

if "%1"=="" (
    echo 用法: create_feature.bat [功能名称]
    echo.
    echo 示例:
    echo   create_feature.bat image-filter
    echo   create_feature.bat batch-rename
    echo   create_feature.bat ui-improvement
    echo.
    pause
    exit /b 1
)

set FEATURE_NAME=%1

echo [1/4] 切换到 develop 分支...
git checkout develop
if errorlevel 1 (
    echo 错误：无法切换到 develop 分支
    pause
    exit /b 1
)
echo.

echo [2/4] 拉取最新代码...
git pull origin develop
echo.

echo [3/4] 创建功能分支: feature/%FEATURE_NAME%
git checkout -b feature/%FEATURE_NAME%
if errorlevel 1 (
    echo 错误：分支创建失败
    pause
    exit /b 1
)
echo.

echo [4/4] 推送到远程仓库...
git push -u origin feature/%FEATURE_NAME%
echo.

echo ========================================
echo   功能分支创建成功！
echo ========================================
echo.
echo 当前分支: feature/%FEATURE_NAME%
echo.
echo 开发流程：
echo   1. 编写代码
echo   2. git add .
echo   3. git commit -m "描述"
echo   4. git push
echo.
echo 完成后合并：
echo   git checkout develop
echo   git merge feature/%FEATURE_NAME%
echo   git push origin develop
echo   git branch -d feature/%FEATURE_NAME%
echo.
echo ========================================
pause

