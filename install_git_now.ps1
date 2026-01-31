# 快速下载并安装 Git
Write-Host "================================" -ForegroundColor Cyan
Write-Host "Git 自动安装程序" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

$url = "https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe"
$installer = "$env:TEMP\git-installer.exe"

Write-Host "步骤 1: 下载 Git..." -ForegroundColor Yellow

try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -Uri $url -OutFile $installer -ErrorAction Stop
    Write-Host "✅ 下载完成" -ForegroundColor Green
} catch {
    Write-Host "❌ 下载失败: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "步骤 2: 安装 Git..." -ForegroundColor Yellow
Write-Host "请稍候 30-60 秒..." -ForegroundColor Cyan

try {
    Start-Process $installer -ArgumentList '/SILENT /NORESTART' -PassThru -Wait -ErrorAction Stop
    Write-Host "✅ 安装完成" -ForegroundColor Green
} catch {
    Write-Host "❌ 安装失败: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "步骤 3: 验证安装..." -ForegroundColor Yellow

Start-Sleep -Seconds 2
$gitPath = "C:\Program Files\Git\bin\git.exe"

if (Test-Path $gitPath) {
    $version = & $gitPath --version 2>&1
    Write-Host "✅ Git 安装成功！" -ForegroundColor Green
    Write-Host "   版本: $version" -ForegroundColor Gray
} else {
    Write-Host "⚠️ Git 可能未完全安装，请重启 PowerShell 后重试" -ForegroundColor Yellow
}

# 清理
Remove-Item $installer -Force -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "✅ 安装完成！请运行下一个脚本开始上传。" -ForegroundColor Green
Write-Host ""
