# Git 自动安装脚本
Write-Host "开始下载和安装 Git..." -ForegroundColor Green

# 下载 Git 安装程序
$gitUrl = "https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe"
$installerPath = "$env:TEMP\Git-installer.exe"

Write-Host "正在下载 Git..." -ForegroundColor Yellow
try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -Uri $gitUrl -OutFile $installerPath -ErrorAction Stop
    Write-Host "下载完成" -ForegroundColor Green
} catch {
    Write-Host "下载失败: $_" -ForegroundColor Red
    Write-Host "请访问 https://git-scm.com/download/win 手动下载安装" -ForegroundColor Yellow
    exit 1
}

# 安装 Git
Write-Host "正在安装 Git..." -ForegroundColor Yellow
try {
    & $installerPath /SILENT /NORESTART
    Start-Sleep -Seconds 30
    Write-Host "Git 安装完成" -ForegroundColor Green
} catch {
    Write-Host "安装失败: $_" -ForegroundColor Red
    exit 1
}

# 清理
Remove-Item $installerPath -Force -ErrorAction SilentlyContinue

Write-Host "请重启 PowerShell 后重新运行上传脚本" -ForegroundColor Green
