# 📤 项目上传指南

## 快速开始

本项目已准备上传到 GitHub，以下是完整的上传步骤。

### 前提条件

1. **安装 Git**
   - 如果未安装，运行 `install_git.ps1` 脚本自动安装
   - 或访问 https://git-scm.com/download/win 手动下载安装

2. **GitHub 账户**
   - 需要有 GitHub 账户
   - 仓库地址：https://github.com/yishihon8-source/imgtool.git

3. **身份验证**
   - GitHub 个人访问令牌 (PAT) 或 SSH 密钥

---

## 上传步骤

### 方法 1：使用 PowerShell 脚本（推荐）

**最简单的方式**

```powershell
# 1. 打开 PowerShell 终端
# 2. 进入项目目录
cd f:\oneDrive\Desktop\ps-cpp

# 3. 运行上传脚本
.\upload_to_github.ps1

# 4. 按提示输入提交信息
# 5. 如果提示需要身份验证，按照下面的说明配置
```

### 方法 2：使用批处理脚本

```bash
# 1. 直接运行批处理文件
f:\oneDrive\Desktop\ps-cpp\upload_to_github.bat

# 2. 按提示输入提交信息和身份验证信息
```

### 方法 3：手动命令行操作

```bash
# 1. 进入项目目录
cd f:\oneDrive\Desktop\ps-cpp

# 2. 初始化 git 仓库（首次）
git init
git config user.name "Your Name"
git config user.email "your@email.com"

# 3. 添加远程仓库
git remote add origin https://github.com/yishihon8-source/imgtool.git

# 4. 添加所有文件
git add -A

# 5. 提交
git commit -m "Initial commit"

# 6. 推送
git push -u origin master
```

---

## 身份验证方式

### 方式 1：GitHub 个人访问令牌 (推荐)

**生成令牌：**
1. 访问 https://github.com/settings/tokens
2. 点击 "Generate new token" → "Generate new token (classic)"
3. 选择以下权限：
   - `repo` - 完全访问公开和私有仓库
4. 生成并复制令牌（只显示一次）

**使用令牌：**
```bash
# 当 git push 提示输入密码时，输入令牌而不是密码
git push -u origin master
# 用户名: your-github-username
# 密码: ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxx (你的令牌)
```

或在 URL 中直接使用：
```bash
git remote set-url origin https://your-username:ghp_xxxxxx@github.com/yishihon8-source/imgtool.git
git push -u origin master
```

### 方式 2：SSH 密钥

**生成 SSH 密钥：**
```bash
ssh-keygen -t rsa -b 4096 -C "your@email.com"
# 按提示保存到默认位置
```

**添加到 GitHub：**
1. 访问 https://github.com/settings/keys
2. 点击 "New SSH key"
3. 复制 `~/.ssh/id_rsa.pub` 的内容

**使用 SSH：**
```bash
# 更改远程 URL 为 SSH 格式
git remote set-url origin git@github.com:yishihon8-source/imgtool.git

# 推送（无需输入密码）
git push -u origin master
```

### 方式 3：GitHub CLI

**安装 GitHub CLI：**
```bash
choco install gh -y
# 或访问 https://cli.github.com 下载
```

**使用 GitHub CLI：**
```bash
# 1. 登录 GitHub
gh auth login
# 选择 GitHub.com
# 选择 HTTPS
# 按提示完成身份验证

# 2. 推送（自动处理身份验证）
git push -u origin master
```

---

## 常见问题

### Q1: 推送时提示 "Authentication failed"

**解决方案：**
1. 确保使用了正确的令牌或 SSH 密钥
2. 检查远程 URL 是否正确
3. 使用 `git remote -v` 查看当前配置

```bash
# 检查远程仓库
git remote -v

# 重新配置（使用个人访问令牌）
git remote set-url origin https://username:token@github.com/yishihon8-source/imgtool.git
git push -u origin master
```

### Q2: 提示 "Permission denied (publickey)"

**解决方案：**
- 这是 SSH 密钥问题
- 检查 SSH 公钥是否已添加到 GitHub
- 或改用 HTTPS 和个人访问令牌

```bash
# 改用 HTTPS
git remote set-url origin https://github.com/yishihon8-source/imgtool.git
```

### Q3: 提示 "fatal: remote origin already exists"

**解决方案：**
- 使用 `set-url` 而不是 `add`
```bash
git remote set-url origin https://github.com/yishihon8-source/imgtool.git
```

### Q4: 需要覆盖远程分支

**解决方案：**
```bash
# 强制推送（谨慎使用）
git push -u origin master --force
```

### Q5: Git 未被识别为命令

**解决方案：**
1. 运行 `install_git.ps1` 自动安装
2. 或从 https://git-scm.com/download/win 手动安装
3. 安装后重启 PowerShell/CMD

---

## 项目文件说明

### 上传脚本
- `install_git.ps1` - Git 自动安装脚本
- `upload_to_github.ps1` - PowerShell 上传脚本（推荐）
- `upload_to_github.bat` - 批处理上传脚本
- `UPLOAD_GUIDE.md` - 本文档

### 项目源代码
- `src/` - 源代码目录
- `third_party/` - 第三方库
- `resources/` - 资源文件
- `CMakeLists.txt` - CMake 构建配置
- `.gitignore` - Git 忽略文件列表

---

## 上传后的仓库访问

上传完成后，您可以通过以下链接访问您的仓库：

```
https://github.com/yishihon8-source/imgtool
```

### 验证上传成功

```bash
# 查看远程仓库
git remote -v

# 查看提交历史
git log --oneline

# 查看当前分支
git branch -a
```

---

## 下次更新

当项目有更新时，只需：

```bash
cd f:\oneDrive\Desktop\ps-cpp
git add -A
git commit -m "描述你的更改"
git push
```

---

## 获取帮助

- **Git 文档**: https://git-scm.com/doc
- **GitHub 帮助**: https://docs.github.com
- **GitHub CLI**: https://cli.github.com/manual

---

**祝上传顺利！** 🚀
