# 快速开始指南

## 🚀 5 分钟快速上手

### 第一步：安装工具

你需要安装以下工具（只需安装一次）：

#### 1. 安装 CMake
- 访问：https://cmake.org/download/
- 下载 "Windows x64 Installer"
- 安装时**务必勾选** "Add CMake to the system PATH"
- 安装完成后**重启命令提示符**

#### 2. 安装 Visual Studio（推荐）
- 访问：https://visualstudio.microsoft.com/zh-hans/downloads/
- 下载 "Visual Studio 2022 Community"（免费）
- 安装时选择工作负载：**"使用 C++ 的桌面开发"**
- 等待安装完成（可能需要 10-20 分钟）

#### 3. 验证安装
打开命令提示符（CMD），输入：
```cmd
cmake --version
```
如果显示版本号，说明安装成功。

---

### 第二步：下载依赖库

在项目目录下，双击运行：
```
download_dependencies.bat
```

或在命令提示符中运行：
```cmd
download_dependencies.bat
```

等待下载完成（需要网络连接）。

---

### 第三步：构建项目

双击运行：
```
build.bat
```

或在命令提示符中运行：
```cmd
build.bat
```

等待编译完成（首次编译可能需要 5-10 分钟）。

---

### 第四步：运行程序

编译成功后，可执行文件位于：
```
build\bin\Release\ImageBatchTool.exe
```

双击运行即可！

---

## 📝 使用说明

### 1. 导入图片
- 点击左侧面板的 "添加图片" 或 "添加文件夹"
- 或直接拖拽图片到窗口（TODO）

### 2. 设置画布
- 在右侧 "画布设置" 中设置输出尺寸
- 选择背景颜色（白色/黑色/透明）

### 3. 调整图片
- 在中间预览区域查看效果
- 可以设置裁剪、缩放、对齐方式

### 4. 批量处理
- 菜单栏 → 处理 → 开始批量处理
- 选择输出文件夹
- 等待处理完成

---

## ❓ 遇到问题？

### 问题 1: "找不到 CMake"
**解决**: 
- 确保已安装 CMake
- 重启命令提示符
- 检查环境变量 PATH 中是否包含 CMake

### 问题 2: "找不到编译器"
**解决**:
- 确保已安装 Visual Studio
- 使用 "x64 Native Tools Command Prompt for VS 2022"
- 或修改 build.bat 中的生成器版本

### 问题 3: 依赖库下载失败
**解决**:
- 检查网络连接
- 确保已安装 Git
- 手动下载（参考 INSTALL.md）

### 问题 4: 编译失败
**解决**:
- 删除 build 文件夹
- 重新运行 build.bat
- 查看错误信息

---

## 🎯 下一步

- 查看 [README.md](README.md) 了解更多功能
- 查看 [INSTALL.md](INSTALL.md) 了解详细安装说明
- 修改源代码进行自定义开发

---

## 💡 提示

- 首次构建时间较长，请耐心等待
- 建议使用 Release 模式（性能更好）
- 如果修改了代码，只需重新运行 build.bat
- 不需要每次都下载依赖库

---

祝你使用愉快！🎉
