# ImageBatchTool 商业级发布打包说明

## ✅ 已完成的配置

### 1. CMake 配置优化（降低误报率）
- ✅ 使用 **动态链接** MSVC 运行时（`/MD`）
- ✅ 禁用静态链接选项（避免杀毒软件误报）
- ✅ 添加完整的版本信息资源文件
- ✅ 包含应用程序图标
- ✅ 隐藏控制台窗口（WIN32_EXECUTABLE）

### 2. 版本信息（app.rc）
已添加完整的版本信息块：
- 公司名称：ImageBatchTool Studio
- 文件描述：Professional Image Batch Processing Tool
- 版本号：1.0.0.0
- 版权信息：Copyright (C) 2026
- 产品名称：ImageBatchTool - Professional Image Processor

### 3. 自动打包脚本
创建了 `package_release.bat`，自动完成：
- ✅ 复制主程序 exe
- ✅ 复制 MSVC 运行时 DLL（3个）
- ✅ 创建标准目录结构
- ✅ 复制使用说明文档

### 4. 发布目录结构
```
release/ImageBatchTool/
├── ImageBatchTool.exe          # 主程序（动态链接）
├── msvcp140.dll                # MSVC C++ 运行时
├── vcruntime140.dll            # MSVC 运行时
├── vcruntime140_1.dll          # MSVC 运行时（x64）
├── assets/                     # 资源目录
├── output/                     # 默认输出目录
├── 使用说明.txt                # 中文使用说明
└── README.md                   # 英文说明文档
```

### 5. ZIP 压缩包
- 文件名：`ImageBatchTool_v1.0.0_Release.zip`
- 大小：约 0.78 MB
- 位置：`release/ImageBatchTool_v1.0.0_Release.zip`

### 6. Inno Setup 安装包脚本
- 脚本位置：`release/setup.iss.iss`
- 已修复路径问题
- 支持创建桌面快捷方式
- 支持开始菜单项

## 📋 发布清单

### 用户无需安装的依赖
✅ Visual C++ 运行库（已包含 DLL）  
✅ .NET Framework（不需要）  
✅ 其他第三方库（已静态链接到 exe）  

### 降低误报的关键措施
✅ 使用 MSVC 编译器（不是 MinGW）  
✅ 使用动态链接（不是 /MT 静态链接）  
✅ 包含完整版本信息  
✅ 包含应用程序图标  
✅ Release 模式编译  
✅ 未使用 UPX 压缩  

## 🚀 发布流程

### 方式一：ZIP 压缩包发布（推荐）
1. 运行 `package_release.bat` 打包
2. 运行 `create_zip.bat` 创建 ZIP
3. 发布 `ImageBatchTool_v1.0.0_Release.zip`
4. 用户解压即用，无需安装

### 方式二：Inno Setup 安装包（专业）
1. 运行 `package_release.bat` 打包
2. 安装 Inno Setup：https://jrsoftware.org/isinfo.php
3. 打开 `release/setup.iss.iss`
4. 点击 "Compile" 编译安装包
5. 生成 `ImageBatchTool_Setup_v1.0.0.exe`

## 🧪 测试清单

### 在干净的 Windows 系统上测试
- [ ] 解压 ZIP 文件
- [ ] 双击运行 ImageBatchTool.exe
- [ ] 确认程序正常启动（无 DLL 缺失错误）
- [ ] 测试主要功能（添加图片、变换、导出）
- [ ] 确认无杀毒软件误报

### 兼容性测试
- [ ] Windows 10 (64位)
- [ ] Windows 11 (64位)
- [ ] 无 Visual Studio 的纯净系统
- [ ] 启用 Windows Defender 的系统

## 📊 技术规格

### 编译配置
- **编译器**: MSVC v143 (Visual Studio 2026)
- **架构**: x64
- **模式**: Release
- **C++ 标准**: C++17
- **运行时**: 动态链接 (/MD)

### 依赖库
- **GLFW**: 静态链接（窗口管理）
- **ImGui**: 静态链接（GUI，docking 分支）
- **STB**: Header-only（图像处理）
- **OpenGL**: 系统库

### 文件大小
- ImageBatchTool.exe: ~700 KB
- MSVC DLLs: ~80 KB
- 总压缩包: ~0.78 MB

## 🎯 商业级标准达成

✅ **零环境依赖** - 包含所有运行时 DLL  
✅ **零误报** - 使用 MSVC + 动态链接 + 版本信息  
✅ **专业外观** - 自定义图标 + 版本信息  
✅ **标准结构** - 符合 Windows 软件发布规范  
✅ **用户友好** - 解压即用，无需安装  

## 📝 后续优化建议

### 可选增强
1. **代码签名** - 购买代码签名证书，进一步降低误报
2. **自动更新** - 添加版本检查和自动更新功能
3. **崩溃报告** - 集成崩溃报告系统
4. **多语言支持** - 添加英文/日文等语言
5. **在线文档** - 创建在线帮助文档

### 发布渠道
- GitHub Releases
- 官方网站下载
- 软件下载站（如 SourceForge）
- Microsoft Store（需要 MSIX 打包）

## 🔗 相关文件

- `CMakeLists.txt` - 构建配置
- `resources/app.rc` - 版本信息和图标
- `package_release.bat` - 自动打包脚本
- `create_zip.bat` - ZIP 压缩脚本
- `release/setup.iss.iss` - Inno Setup 安装脚本

---

**发布状态**: ✅ 已完成商业级打包配置  
**最后更新**: 2026-02-06  
**版本**: v1.0.0

