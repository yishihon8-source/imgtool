# 完整的 Git 配置和上传自动化脚本
# 功能: 自动检查、安装 Git，初始化仓库，并上传到 GitHub

param(
    [string]$CommitMessage = "Upload ImageBatchTool project",
    [string]$GitHubRepo = "https://github.com/yishihon8-source/imgtool.git"
)

$projectPath = "f:\oneDrive\Desktop\ps-cpp"
$gitExePaths = @(
    "C:\Program Files\Git\bin\git.exe",
    "C:\Program Files (x86)\Git\bin\git.exe",
    "C:\Program Files\Git\cmd\git.exe"
)

Write-Host "╔════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  ImageBatchTool - GitHub 上传工具    ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# 检查 Git
function Test-GitInstalled {
    # 先尝试直接调用 git
    try {
        & git --version 2>&1 | Out-Null
        return $true
    } catch {
    }
    
    # 再尝试检查常见安装位置
    foreach ($path in $gitExePaths) {
        if (Test-Path $path) {
            # 将 Git 路径添加到环境变量（仅限当前进程）
            $gitBinPath = Split-Path $path
            $env:PATH = "$gitBinPath;$env:PATH"
            return $true
        }
    }
    
    return $false
}

Write-Host "📋 检查环境..." -ForegroundColor Yellow

if (Test-GitInstalled) {
    $gitVersion = & git --version 2>&1
    Write-Host "✅ Git 已安装: $gitVersion" -ForegroundColor Green
} else {
    Write-Host "❌ Git 未安装" -ForegroundColor Red
    Write-Host ""
    Write-Host "📥 下载并安装 Git:" -ForegroundColor Yellow
    Write-Host "   1. 访问: https://git-scm.com/download/win" -ForegroundColor Cyan
    Write-Host "   2. 下载 Git for Windows"
    Write-Host "   3. 运行安装程序（选择默认配置）"
    Write-Host "   4. 重启 PowerShell"
    Write-Host "   5. 重新运行本脚本"
    Write-Host ""
    exit 1
}

# 进入项目目录
Push-Location $projectPath
Write-Host ""
Write-Host "📂 项目目录: $projectPath" -ForegroundColor Green

# 初始化或检查 Git 仓库
Write-Host ""
Write-Host "🔧 初始化 Git 配置..." -ForegroundColor Yellow

if (-not (Test-Path ".git")) {
    Write-Host "  初始化新的 Git 仓库..." -ForegroundColor Cyan
    & git init 2>&1 | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
    
    # 配置用户信息
    & git config user.name "ImageBatchTool Developer" 2>&1
    & git config user.email "dev@imgtool.local" 2>&1
    Write-Host "  ✅ 仓库初始化完成" -ForegroundColor Green
} else {
    Write-Host "  ✅ Git 仓库已存在" -ForegroundColor Green
}

# 检查和配置远程仓库
Write-Host ""
Write-Host "🌐 配置远程仓库..." -ForegroundColor Yellow

$remoteUrl = & git config --get remote.origin.url 2>&1

if ($remoteUrl -like "*fatal*" -or $remoteUrl -eq "") {
    Write-Host "  添加远程仓库: $GitHubRepo" -ForegroundColor Cyan
    & git remote add origin $GitHubRepo 2>&1 | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
} else {
    Write-Host "  当前远程仓库: $remoteUrl" -ForegroundColor Green
    if ($remoteUrl -ne $GitHubRepo) {
        Write-Host "  更新远程仓库 URL..." -ForegroundColor Cyan
        & git remote set-url origin $GitHubRepo 2>&1 | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
    }
}

# 显示当前状态
Write-Host ""
Write-Host "📊 仓库状态..." -ForegroundColor Yellow
$unstagedFiles = & git status --porcelain 2>&1

if ($unstagedFiles) {
    Write-Host "  发现 $(($unstagedFiles | Measure-Object).Count) 个文件需要提交:" -ForegroundColor Cyan
    $unstagedFiles | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
} else {
    Write-Host "  所有文件已提交" -ForegroundColor Green
}

# 添加文件
Write-Host ""
Write-Host "📝 添加文件到暂存区..." -ForegroundColor Yellow
& git add -A 2>&1 | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }

# 检查是否有文件要提交
$statusAfterAdd = & git status --porcelain 2>&1
if ($statusAfterAdd) {
    Write-Host "  ✅ 已添加 $(($statusAfterAdd | Measure-Object).Count) 个文件" -ForegroundColor Green
    
    # 提交
    Write-Host ""
    Write-Host "💾 提交更改..." -ForegroundColor Yellow
    $finalMessage = "$CommitMessage - $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    Write-Host "  提交信息: $finalMessage" -ForegroundColor Cyan
    
    & git commit -m "$finalMessage" 2>&1 | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  ✅ 提交成功" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  提交出现问题（可能没有变更）" -ForegroundColor Yellow
    }
} else {
    Write-Host "  ℹ️  没有新文件需要提交" -ForegroundColor Cyan
}

# 推送
Write-Host ""
Write-Host "🚀 推送到 GitHub..." -ForegroundColor Yellow
Write-Host ""
Write-Host "⚠️  身份验证提示:" -ForegroundColor Yellow
Write-Host "   如果要求输入凭据，请输入:" -ForegroundColor Cyan
Write-Host "   用户名: 你的 GitHub 用户名" -ForegroundColor Gray
Write-Host "   密码: 你的 GitHub 个人访问令牌（或 SSH 密钥）" -ForegroundColor Gray
Write-Host ""
Write-Host "   获取令牌步骤:" -ForegroundColor Cyan
Write-Host "   1. 访问 https://github.com/settings/tokens" -ForegroundColor Gray
Write-Host "   2. 点击 'Generate new token (classic)'" -ForegroundColor Gray
Write-Host "   3. 勾选 'repo' 权限" -ForegroundColor Gray
Write-Host "   4. 生成令牌并复制" -ForegroundColor Gray
Write-Host ""

$pushResult = & git push -u origin master 2>&1
$pushSuccess = $LASTEXITCODE -eq 0

if ($pushSuccess) {
    Write-Host "✅ 推送成功！" -ForegroundColor Green
    Write-Host ""
    Write-Host "📍 仓库地址:" -ForegroundColor Green
    Write-Host "   https://github.com/yishihon8-source/imgtool" -ForegroundColor Cyan
} else {
    Write-Host "输出:" -ForegroundColor Yellow
    $pushResult | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }
    
    Write-Host ""
    Write-Host "❌ 推送遇到问题" -ForegroundColor Red
    
    # 尝试诊断
    Write-Host ""
    Write-Host "🔍 故障排除:" -ForegroundColor Yellow
    
    if ($pushResult -like "*Authentication*") {
        Write-Host "   • 身份验证失败 - 请检查用户名和令牌" -ForegroundColor Cyan
        Write-Host "   • 确保令牌有 'repo' 权限" -ForegroundColor Cyan
    }
    
    if ($pushResult -like "*Permission denied*") {
        Write-Host "   • 权限被拒绝 - 检查仓库访问权限" -ForegroundColor Cyan
        Write-Host "   • 或者切换到 HTTPS 方式重试" -ForegroundColor Cyan
    }
    
    if ($pushResult -like "*Could not resolve host*") {
        Write-Host "   • 网络连接问题 - 检查互联网连接" -ForegroundColor Cyan
    }
}

# 显示提交日志
Write-Host ""
Write-Host "📜 最新提交:" -ForegroundColor Yellow
& git log --oneline -5 2>&1 | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }

# 清理
Pop-Location

Write-Host ""
Write-Host "╔════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║         脚本执行完成                   ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

if ($pushSuccess) {
    Write-Host "✨ 项目已成功上传到 GitHub！" -ForegroundColor Green
} else {
    Write-Host "⚠️  请检查上面的错误信息并重试" -ForegroundColor Yellow
}
