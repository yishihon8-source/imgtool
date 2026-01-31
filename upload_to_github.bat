@echo off
REM Git 上传脚本
cd /d f:\oneDrive\Desktop\ps-cpp

REM 检查 Git 是否已安装
git --version >nul 2>&1
if errorlevel 1 (
    echo Git 未安装。请先运行 install_git.ps1
    pause
    exit /b 1
)

echo ======================================
echo 项目上传到 GitHub
echo ======================================
echo.

REM 初始化 git 仓库（如果还没有）
if not exist .git (
    echo 初始化 git 仓库...
    git init
    git config user.name "ImageBatchTool"
    git config user.email "dev@imgtool.local"
)

REM 检查远程仓库是否存在
git remote get-url origin >nul 2>&1
if errorlevel 1 (
    echo 添加远程仓库...
    git remote add origin https://github.com/yishihon8-source/imgtool.git
) else (
    echo 更新远程仓库...
    git remote set-url origin https://github.com/yishihon8-source/imgtool.git
)

REM 添加所有文件
echo 添加文件到暂存区...
git add -A

REM 提交
echo 提交更改...
set /p commit_msg="输入提交信息 (默认: Initial commit): "
if "%commit_msg%"=="" set commit_msg=Initial commit
git commit -m "%commit_msg%"

REM 推送到远程仓库
echo 推送到远程仓库...
echo 需要输入 GitHub 账户信息...
git push -u origin master 2>&1

echo.
echo ======================================
echo 上传完成！
echo ======================================
pause
