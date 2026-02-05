@echo off
chcp 65001 >nul
echo ========================================
echo   创建发布分支 (Release Branch)
echo ========================================
echo.

if "%1"=="" (
    echo 用法: create_release.bat [版本号]
    echo.
    echo 示例:
    echo   create_release.bat v1.1.0
    echo   create_release.bat v2.0.0
    echo.
    pause
    exit /b 1
)

set VERSION=%1

echo [1/5] 切换到 develop 分支...
git checkout develop
if errorlevel 1 (
    echo 错误：无法切换到 develop 分支
    pause
    exit /b 1
)
echo.

echo [2/5] 拉取最新代码...
git pull origin develop
echo.

echo [3/5] 创建发布分支: release/%VERSION%
git checkout -b release/%VERSION%
if errorlevel 1 (
    echo 错误：分支创建失败
    pause
    exit /b 1
)
echo.

echo [4/5] 推送到远程仓库...
git push -u origin release/%VERSION%
echo.

echo [5/5] 显示当前状态...
git status
echo.

echo ========================================
echo   发布分支创建成功！
echo ========================================
echo.
echo 当前分支: release/%VERSION%
echo.
echo 发布流程：
echo   1. 修复最后的 bug
echo   2. 更新版本号和文档
echo   3. 测试验证
echo   4. 合并到 main 和 develop
echo.
echo 完成发布：
echo   git checkout main
echo   git merge release/%VERSION%
echo   git tag -a %VERSION% -m "版本说明"
echo   git push origin main --tags
echo.
echo   git checkout develop
echo   git merge release/%VERSION%
echo   git push origin develop
echo.
echo   git branch -d release/%VERSION%
echo.
echo ========================================
pause

