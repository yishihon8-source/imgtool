# Git Flow 工作流程 - 快速参考

## 📋 分支结构

```
main (稳定发布版本)
  ├── v1.0.0 (tag)
  ├── v1.1.0 (tag)
  └── ...

develop (持续开发版本)
  ├── feature/image-filter
  ├── feature/batch-rename
  └── feature/ui-improvement

release/v1.1.0 (发布准备)

hotfix/critical-bug (紧急修复)
```

## 🚀 快速开始

### 1. 初始化 Git Flow

```bash
# 运行初始化脚本
git_flow_init.bat
```

这会：
- 提交当前更改到 main
- 创建 develop 分支
- 创建 v1.0.0 标签
- 推送到远程仓库

## 🔧 日常开发流程

### 2. 开发新功能

#### 方式一：使用脚本（推荐）
```bash
# 创建功能分支
create_feature.bat image-filter

# 开发代码...
git add .
git commit -m "feat: 添加图片滤镜功能"
git push

# 完成后合并
git checkout develop
git merge feature/image-filter
git push origin develop
git branch -d feature/image-filter
```

#### 方式二：手动操作
```bash
# 从 develop 创建功能分支
git checkout develop
git checkout -b feature/image-filter

# 开发代码...
git add .
git commit -m "feat: 添加图片滤镜功能"

# 完成后合并回 develop
git checkout develop
git merge feature/image-filter
git push origin develop

# 删除功能分支
git branch -d feature/image-filter
```

### 3. 准备发布新版本

#### 方式一：使用脚本（推荐）
```bash
# 创建发布分支
create_release.bat v1.1.0

# 修复最后的 bug、更新文档
git add .
git commit -m "chore: 准备 v1.1.0 发布"

# 合并到 main
git checkout main
git merge release/v1.1.0
git tag -a v1.1.0 -m "v1.1.0 发布说明"
git push origin main --tags

# 合并回 develop
git checkout develop
git merge release/v1.1.0
git push origin develop

# 删除发布分支
git branch -d release/v1.1.0
```

#### 方式二：手动操作
```bash
# 从 develop 创建发布分支
git checkout develop
git checkout -b release/v1.1.0

# 修复 bug、更新版本号
git add .
git commit -m "chore: 准备 v1.1.0 发布"

# 合并到 main
git checkout main
git merge release/v1.1.0

# 打标签
git tag -a v1.1.0 -m "v1.1.0 发布说明"
git push origin main --tags

# 合并回 develop
git checkout develop
git merge release/v1.1.0
git push origin develop

# 删除发布分支
git branch -d release/v1.1.0
```

### 4. 紧急修复（Hotfix）

```bash
# 从 main 创建 hotfix 分支
git checkout main
git checkout -b hotfix/critical-bug

# 修复 bug
git add .
git commit -m "fix: 修复关键 bug"

# 合并到 main
git checkout main
git merge hotfix/critical-bug
git tag -a v1.0.1 -m "紧急修复版本"
git push origin main --tags

# 合并回 develop
git checkout develop
git merge hotfix/critical-bug
git push origin develop

# 删除 hotfix 分支
git branch -d hotfix/critical-bug
```

## 📝 提交信息规范

使用语义化提交信息：

```bash
feat: 新功能
fix: Bug 修复
docs: 文档更新
style: 代码格式（不影响功能）
refactor: 重构
perf: 性能优化
test: 测试相关
chore: 构建/工具相关
```

示例：
```bash
git commit -m "feat: 添加批量重命名功能"
git commit -m "fix: 修复图片加载失败的问题"
git commit -m "docs: 更新 README 使用说明"
git commit -m "perf: 优化图片处理性能"
```

## 🎯 分支命名规范

- **功能分支**: `feature/功能名称`
  - `feature/image-filter`
  - `feature/batch-rename`
  - `feature/ui-improvement`

- **发布分支**: `release/版本号`
  - `release/v1.1.0`
  - `release/v2.0.0`

- **修复分支**: `hotfix/问题描述`
  - `hotfix/critical-bug`
  - `hotfix/memory-leak`

## 📦 发布到 GitHub Release

### 上传发布包

```bash
# 1. 创建标签并推送
git tag -a v1.0.0 -m "v1.0.0 正式发布"
git push origin v1.0.0

# 2. 在 GitHub 上创建 Release
# - 访问仓库的 Releases 页面
# - 点击 "Create a new release"
# - 选择标签 v1.0.0
# - 上传发布文件：
#   - ImageBatchTool_v1.0.0_Release.zip
#   - ImageBatchTool_Setup_v1.0.0.exe
# - 编写 Release Notes
# - 发布
```

### Release Notes 模板

```markdown
## ImageBatchTool v1.0.0

### ✨ 新功能
- Photoshop 级自由变换 (Ctrl+T)
- 批量图片处理
- 支持 JPG/PNG/BMP/TGA 格式
- 可视化画布系统

### 🔧 技术改进
- 使用 MSVC 动态链接
- 零环境依赖
- 零误报风险

### 📦 下载
- **ZIP 压缩包**: ImageBatchTool_v1.0.0_Release.zip (0.78 MB)
  - 绿色便携，解压即用
- **安装包**: ImageBatchTool_Setup_v1.0.0.exe (2.59 MB)
  - 标准安装，自动创建快捷方式

### 📋 系统要求
- Windows 10/11 (64位)
- 无需安装任何依赖

### 📝 更新日志
详见 [CHANGELOG.md](CHANGELOG.md)
```

## 🛠️ 常用命令

```bash
# 查看所有分支
git branch -a

# 查看当前状态
git status

# 查看提交历史
git log --oneline --graph --all

# 查看所有标签
git tag

# 切换分支
git checkout 分支名

# 拉取最新代码
git pull

# 推送代码
git push

# 推送标签
git push --tags
```

## 📚 工作流程总结

| 场景 | 操作 |
|------|------|
| 开发新功能 | `feature` 分支 → `develop` |
| 准备发布 | `release` 分支 → `main` + `develop` |
| 紧急修复 | `hotfix` 分支 → `main` + `develop` |
| 日常开发 | 在 `develop` 分支 |
| 稳定版本 | 在 `main` 分支 |

## 🎓 最佳实践

1. **永远不要直接在 main 分支开发**
2. **功能开发完成后及时合并到 develop**
3. **发布前在 release 分支充分测试**
4. **使用语义化版本号**: v主版本.次版本.修订号
5. **每次发布都打标签**
6. **提交信息要清晰明确**
7. **定期推送到远程仓库**

---

**快速脚本**:
- `git_flow_init.bat` - 初始化 Git Flow
- `create_feature.bat` - 创建功能分支
- `create_release.bat` - 创建发布分支

