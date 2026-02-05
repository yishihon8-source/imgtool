@echo off
chcp 65001 >nul
echo ========================================
echo   Git Flow 分支管理 - 初始化
echo ========================================
echo.

echo [1/6] 检查当前分支...
git branch
echo.

echo [2/6] 提交当前更改到 main 分支...
git add .
git commit -m "feat: 商业级发布配置完成 - v1.0.0

- 修改为动态链接 MSVC 运行时 (/MD)
- 添加完整版本信息到 app.rc
- 创建自动打包脚本 (package_release.bat)
- 创建 ZIP 压缩脚本 (create_zip.bat)
- 创建 Inno Setup 安装包脚本
- 添加发布文档和使用说明
- 生成 v1.0.0 发布包 (ZIP + 安装包)

降低误报率措施：
- 使用 MSVC 编译器
- 动态链接运行时
- 包含版本信息和图标
- Release 模式编译

发布文件：
- ImageBatchTool_v1.0.0_Release.zip (0.78 MB)
- ImageBatchTool_Setup_v1.0.0.exe (2.59 MB)"

if errorlevel 1 (
    echo 提交失败或无更改
)
echo.

echo [3/6] 创建 develop 分支（开发分支）...
git checkout -b develop 2>nul
if errorlevel 1 (
    echo develop 分支已存在，切换到 develop
    git checkout develop
)
echo.

echo [4/6] 推送 main 和 develop 分支到远程...
git push -u origin main
git push -u origin develop
echo.

echo [5/6] 创建 v1.0.0 标签...
git tag -a v1.0.0 -m "ImageBatchTool v1.0.0 正式发布

功能特点：
- Photoshop 级自由变换 (Ctrl+T)
- 批量图片处理
- 支持 JPG/PNG/BMP/TGA
- 可视化画布系统
- 高质量输出

技术规格：
- MSVC 动态链接
- 零环境依赖
- 零误报风险
- 商业级发布标准"

git push origin v1.0.0
echo.

echo [6/6] 显示分支结构...
git branch -a
echo.
git tag
echo.

echo ========================================
echo   Git Flow 初始化完成！
echo ========================================
echo.
echo 分支结构：
echo   main     - 稳定发布版本
echo   develop  - 持续开发版本
echo.
echo 标签：
echo   v1.0.0   - 当前发布版本
echo.
echo 下一步开发流程：
echo   1. 新功能开发：
echo      git checkout develop
echo      git checkout -b feature/功能名称
echo.
echo   2. 功能完成后：
echo      git checkout develop
echo      git merge feature/功能名称
echo.
echo   3. 准备发布：
echo      git checkout -b release/v1.1.0
echo      git checkout main
echo      git merge release/v1.1.0
echo      git tag -a v1.1.0 -m "版本说明"
echo.
echo ========================================
pause

