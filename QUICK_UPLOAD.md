# 🚀 快速上传到 GitHub - 3 步完成

## ⚡ 快速开始（5 分钟）

### 第 1 步：安装 Git（2 分钟）

在 PowerShell（以管理员身份）中运行：

```powershell
$url = "https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe"
$installer = "$env:TEMP\git-installer.exe"
Invoke-WebRequest -Uri $url -OutFile $installer
Start-Process $installer -ArgumentList '/SILENT /NORESTART' -Wait
```

安装完成后，**重启 PowerShell**。

### 第 2 步：生成 GitHub 令牌（2 分钟）

1. 进入：https://github.com/settings/tokens/new
2. 在 **Note** 字段输入：`Git-ImageBatchTool`
3. 勾选 ✅ **repo** 权限
4. 点击 **Generate token**
5. **复制令牌**（形如：`ghp_xxxxxxxxxxxxxxxxxxxx`）

### 第 3 步：上传项目（1 分钟）

```powershell
cd f:\oneDrive\Desktop\ps-cpp

# 运行自动上传脚本
.\auto_push_to_github.ps1
```

按照提示输入：
- 用户名：你的 GitHub 用户名
- 密码：粘贴第 2 步复制的令牌

**完成！** ✅ 项目已在 GitHub 上：https://github.com/yishihon8-source/imgtool

---

## 📝 手动上传（如果脚本不工作）

```powershell
cd f:\oneDrive\Desktop\ps-cpp

git init
git config user.name "Developer"
git config user.email "dev@example.com"
git remote add origin https://github.com/yishihon8-source/imgtool.git
git add -A
git commit -m "Initial commit"
git push -u origin master
```

---

## 常见问题

### ❓ 脚本运行错误 "无法将 git 项识别..."

**解决**：重启 PowerShell 并确保已安装 Git
```powershell
git --version  # 应该显示版本号
```

### ❓ 推送时提示 "Authentication failed"

**解决**：确保使用的是令牌而不是密码
```powershell
# 清除旧凭据
git credential reject

# 重新推送（输入令牌）
git push
```

### ❓ 仓库已存在要覆盖

**解决**：
```powershell
git remote set-url origin https://github.com/yishihon8-source/imgtool.git
git push -u origin master --force
```

---

## ✅ 验证上传成功

```powershell
# 检查远程配置
git remote -v

# 查看提交
git log --oneline
```

---

💡 **需要详细帮助？** 查看 `GITHUB_UPLOAD_COMPLETE_GUIDE.md`
