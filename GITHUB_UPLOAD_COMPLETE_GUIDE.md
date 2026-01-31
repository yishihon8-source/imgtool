# 📤 项目上传到 GitHub - 完整指南

## 当前状态

✅ 项目已准备好上传到 GitHub  
📍 目标仓库：https://github.com/yishihon8-source/imgtool.git  
⚙️ 状态：等待 Git 配置

---

## 第一步：安装 Git

### 自动安装（推荐）

在项目目录中，使用 PowerShell 运行以下命令：

```powershell
# 1. 以管理员身份打开 PowerShell
# 2. 运行以下命令

# 下载 Git 安装程序
$url = "https://github.com/git-for-windows/git/releases/download/v2.43.0.windows.1/Git-2.43.0-64-bit.exe"
$installer = "$env:TEMP\git-installer.exe"
Invoke-WebRequest -Uri $url -OutFile $installer

# 安装 Git（无需交互）
Start-Process $installer -ArgumentList '/SILENT /NORESTART' -Wait

# 刷新环境变量
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("PATH","User")

# 验证安装
git --version
```

### 手动安装

如果自动安装失败，请手动下载和安装：

1. 访问：https://git-scm.com/download/win
2. 点击 "Click here to download" 下载 Git for Windows
3. 运行安装程序，选择以下选项：
   - ✅ 添加 Git 到系统 PATH
   - ✅ 使用 Windows 默认换行符转换
   - ✅ 使用 MinTTY 作为默认终端
4. 完成安装后，重启 PowerShell 或 CMD

---

## 第二步：生成 GitHub 认证

### 方式 A：个人访问令牌（推荐）

**优点**：安全、可随时撤销、可限制权限

**步骤：**

1. 登录 GitHub：https://github.com

2. 进入设置 → 开发者设置 → 个人访问令牌：
   - 访问链接：https://github.com/settings/tokens

3. 点击 "Generate new token (classic)"

4. 填写信息：
   - **Note**：输入 "Git CLI - ImageBatchTool"
   - **Expiration**：选择 "No expiration" 或 "30 days"

5. 选择权限：
   - ✅ 勾选 `repo` - 完全访问公开和私有仓库

6. 点击 "Generate token"

7. **重要**：复制生成的令牌（只显示一次）
   ```
   ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxx
   ```

### 方式 B：SSH 密钥（高级）

**步骤：**

```powershell
# 1. 生成 SSH 密钥
ssh-keygen -t rsa -b 4096 -C "your@email.com"
# 按 Enter 三次使用默认设置

# 2. 查看公钥内容
Get-Content ~/.ssh/id_rsa.pub | Set-Clipboard
# 或者手动查看 C:\Users\YourName\.ssh\id_rsa.pub

# 3. 在 GitHub 添加 SSH 公钥
# 访问：https://github.com/settings/keys
# 点击 "New SSH key"
# 粘贴上面复制的公钥内容
# 保存
```

---

## 第三步：配置并上传项目

### 自动上传（一键推送）

安装 Git 后，在项目目录中运行：

```powershell
cd f:\oneDrive\Desktop\ps-cpp

# 运行自动上传脚本
.\auto_push_to_github.ps1
```

**脚本会自动：**
- ✅ 初始化 Git 仓库
- ✅ 配置远程仓库
- ✅ 添加所有文件
- ✅ 提交更改
- ✅ 推送到 GitHub

### 手动上传步骤

```powershell
cd f:\oneDrive\Desktop\ps-cpp

# 1. 初始化 Git 仓库
git init

# 2. 配置用户信息
git config user.name "Your Name"
git config user.email "your@email.com"

# 3. 添加远程仓库
git remote add origin https://github.com/yishihon8-source/imgtool.git

# 4. 添加所有文件
git add -A

# 5. 提交
git commit -m "Initial commit - ImageBatchTool project"

# 6. 推送到 GitHub
git push -u origin master

# 当提示输入凭据时：
# 用户名：your-github-username
# 密码：你的个人访问令牌（ghp_xxxxx...）
```

---

## 故障排除

### 问题 1: "git: 无法将 git 项识别为命令"

**原因**：Git 未安装或未添加到 PATH

**解决**：
```powershell
# 方案 A：重启 PowerShell 后重试
# 关闭 PowerShell 后重新打开

# 方案 B：手动添加 Git 到 PATH
$env:PATH = "C:\Program Files\Git\bin;$env:PATH"
git --version

# 方案 C：检查 Git 安装位置
Test-Path "C:\Program Files\Git\bin\git.exe"
```

### 问题 2: "fatal: Authentication failed"

**原因**：用户名或令牌错误

**解决**：
```powershell
# 清除之前保存的凭据
git credential reject
# 或者手动在 URL 中提供令牌
git remote set-url origin "https://your-username:ghp_token@github.com/yishihon8-source/imgtool.git"

# 重新推送
git push -u origin master
```

### 问题 3: "Permission denied (publickey)"

**原因**：SSH 密钥配置错误

**解决**：改用 HTTPS 和个人访问令牌
```powershell
git remote set-url origin https://github.com/yishihon8-source/imgtool.git
git push -u origin master
```

### 问题 4: "fatal: remote origin already exists"

**原因**：远程仓库已配置

**解决**：更新而不是添加
```powershell
git remote set-url origin https://github.com/yishihon8-source/imgtool.git
```

### 问题 5: "You don't have permission to push to this repository"

**原因**：仓库权限问题或令牌权限不足

**解决**：
1. 确保有仓库的写入权限
2. 检查令牌是否勾选了 `repo` 权限
3. 如果令牌过期，重新生成新令牌

---

## 验证上传成功

上传完成后，验证：

```powershell
# 1. 查看远程仓库
git remote -v

# 2. 查看提交历史
git log --oneline -5

# 3. 查看当前分支
git branch -a
```

**预期输出：**
```
origin  https://github.com/yishihon8-source/imgtool.git (fetch)
origin  https://github.com/yishihon8-source/imgtool.git (push)

* master
```

---

## 后续更新

当项目有更新时，只需：

```powershell
cd f:\oneDrive\Desktop\ps-cpp

# 查看更改
git status

# 提交更新
git add -A
git commit -m "描述你的更改"
git push
```

---

## 项目上传的文件清单

### 源代码目录
- `src/` - 完整的项目源代码
  - `main.cpp` - 程序入口
  - `app/` - 应用框架（App.h/cpp）
  - `core/` - 核心模块（选区系统、变换管理、图像处理等）
  - `ui/` - UI 模块（主界面、预览面板、控制面板等）
  - `task/` - 任务调度（线程池、批处理）
  - `utils/` - 工具类（文件对话框、日志等）

### 配置文件
- `CMakeLists.txt` - CMake 构建配置
- `.gitignore` - Git 忽略文件列表

### 第三方库
- `third_party/` - 包含 imgui, glfw, stb 等库

### 文档
- `README.md` - 项目说明
- `UPLOAD_GUIDE.md` - 本上传指南
- 各种功能设计文档（RENAME_FIX.md, SELECTION_SUMMARY.md 等）

---

## 快速参考

| 命令 | 说明 |
|------|------|
| `git init` | 初始化仓库 |
| `git add -A` | 添加所有文件 |
| `git commit -m "msg"` | 提交 |
| `git push` | 推送到远程 |
| `git pull` | 从远程拉取 |
| `git status` | 查看状态 |
| `git log` | 查看历史 |

---

## 需要帮助？

- **GitHub 文档**：https://docs.github.com
- **Git 教程**：https://git-scm.com/book/zh/v2
- **Git 命令参考**：https://git-scm.com/docs

---

**祝上传顺利！** 🚀
