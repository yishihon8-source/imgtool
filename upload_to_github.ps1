# Git 项目上传脚本
# 使用方法: .\upload_to_github.ps1

$projectPath = "f:\oneDrive\Desktop\ps-cpp"
$gitUrl = "https://github.com/yishihon8-source/imgtool.git"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "项目上传到 GitHub" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# 检查 Git 是否已安装
try {
    $gitVersion = & git --version 2>&1
    Write-Host "Git 版本: $gitVersion" -ForegroundColor Green
} catch {
    Write-Host "Git 未安装！" -ForegroundColor Red
    Write-Host "请先运行 install_git.ps1 脚本安装 Git" -ForegroundColor Yellow
    exit 1
}

# 进入项目目录
Push-Location $projectPath

# 初始化 git 仓库（如果还没有）
if (-not (Test-Path ".git")) {
    Write-Host "初始化 git 仓库..." -ForegroundColor Yellow
    git init
    git config user.name "ImageBatchTool"
    git config user.email "dev@imgtool.local"
    Write-Host "Git 仓库初始化完成" -ForegroundColor Green
} else {
    Write-Host "Git 仓库已存在" -ForegroundColor Green
}

# 检查远程仓库配置
$remoteUrl = & git remote get-url origin 2>&1
if ($LASTEXITCODE -ne 0 -or $remoteUrl -like "*fatal*") {
    Write-Host "添加远程仓库..." -ForegroundColor Yellow
    git remote add origin $gitUrl
    Write-Host "远程仓库添加完成: $gitUrl" -ForegroundColor Green
} else {
    Write-Host "远程仓库已存在: $remoteUrl" -ForegroundColor Green
    if ($remoteUrl -ne $gitUrl) {
        Write-Host "更新远程仓库 URL..." -ForegroundColor Yellow
        git remote set-url origin $gitUrl
        Write-Host "远程仓库 URL 已更新" -ForegroundColor Green
    }
}

# 查看当前状态
Write-Host ""
Write-Host "当前仓库状态:" -ForegroundColor Yellow
git status --short

# 添加所有文件
Write-Host ""
Write-Host "添加所有文件到暂存区..." -ForegroundColor Yellow
git add -A
Write-Host "文件添加完成" -ForegroundColor Green

# 检查是否有文件要提交
$status = & git status --porcelain
if ($status) {
    # 提交
    Write-Host ""
    Write-Host "请输入提交信息" -ForegroundColor Yellow
    $commitMsg = Read-Host "提交信息 (默认: Update project files)"
    if ([string]::IsNullOrWhiteSpace($commitMsg)) {
        $commitMsg = "Update project files on $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    }
    
    Write-Host "提交更改..." -ForegroundColor Yellow
    git commit -m "$commitMsg"
    Write-Host "提交完成" -ForegroundColor Green
} else {
    Write-Host "没有文件需要提交" -ForegroundColor Green
}

# 推送到远程仓库
Write-Host ""
Write-Host "准备推送到远程仓库..." -ForegroundColor Yellow
Write-Host "提示: 如果仓库需要身份验证，请使用以下任一方式:" -ForegroundColor Cyan
Write-Host "  1. GitHub 个人访问令牌 (Personal Access Token)" -ForegroundColor Cyan
Write-Host "  2. SSH 密钥" -ForegroundColor Cyan
Write-Host "  3. 在 URL 中直接使用: https://username:token@github.com/yishihon8-source/imgtool.git" -ForegroundColor Cyan
Write-Host ""

# 尝试推送
try {
    Write-Host "推送到 GitHub..." -ForegroundColor Yellow
    $output = & git push -u origin master 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "推送成功！" -ForegroundColor Green
        Write-Host ""
        Write-Host "您的项目已上传到:" -ForegroundColor Green
        Write-Host "$gitUrl" -ForegroundColor Cyan
    } else {
        Write-Host "推送遇到问题:" -ForegroundColor Red
        Write-Host $output
        
        Write-Host ""
        Write-Host "常见问题：" -ForegroundColor Yellow
        Write-Host "1. 如果提示 'Authentication failed'，需要配置身份验证" -ForegroundColor Yellow
        Write-Host "   可以使用 GitHub CLI 或配置 SSH 密钥" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "2. 如果需要覆盖远程分支，可以使用:" -ForegroundColor Yellow
        Write-Host "   git push -u origin master --force" -ForegroundColor Gray
    }
} catch {
    Write-Host "推送失败: $_" -ForegroundColor Red
}

Pop-Location

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "上传脚本执行完成" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
