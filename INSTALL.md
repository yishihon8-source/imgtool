# ImageBatchTool 安装与构建指南

## 前置要求

在构建本项目之前，需要安装以下工具：

### 1. CMake（必需）

**下载地址**: https://cmake.org/download/

**推荐版本**: 3.15 或更高

**安装步骤**:
1. 下载 Windows x64 Installer
2. 运行安装程序
3. **重要**: 安装时勾选 "Add CMake to the system PATH"
4. 安装完成后，重启命令提示符

**验证安装**:
```cmd
cmake --version
```

### 2. C++ 编译器（选择其一）

#### 选项 A: Visual Studio（推荐）

**下载地址**: https://visualstudio.microsoft.com/zh-hans/downloads/

**推荐版本**: Visual Studio 2019 或 2022 Community（免费）

**安装步骤**:
1. 下载 Visual Studio Installer
2. 运行安装程序
3. 选择工作负载：**"使用 C++ 的桌面开发"**
4. 安装完成后重启

#### 选项 B: MinGW-w64

**下载地址**: https://www.mingw-w64.org/downloads/

或使用 MSYS2: https://www.msys2.org/

**安装步骤**:
1. 下载并安装 MSYS2
2. 打开 MSYS2 终端，运行：
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake
   ```
3. 将 MinGW bin 目录添加到系统 PATH

### 3. Git（必需，用于下载依赖）

**下载地址**: https://git-scm.com/download/win

**安装步骤**:
1. 下载 Git for Windows
2. 运行安装程序（使用默认设置即可）

**验证安装**:
```cmd
git --version
```

---

## 快速开始

### 步骤 1: 下载依赖库

```cmd
download_dependencies.bat
```

这将自动下载：
- ImGui（UI 库）
- GLFW（窗口库）
- STB（图像库）

### 步骤 2: 构建项目

#### 使用 Visual Studio:

```cmd
build.bat
```

或手动构建：

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### 使用 MinGW:

```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### 步骤 3: 运行程序

```cmd
cd build\bin\Release
ImageBatchTool.exe
```

---

## 常见问题

### Q1: CMake 提示找不到编译器

**解决方案**:
- 确保已安装 Visual Studio 或 MinGW
- 对于 Visual Studio，使用 "x64 Native Tools Command Prompt"
- 对于 MinGW，确保 bin 目录在 PATH 中

### Q2: 编译时出现 "cannot open file 'imgui.h'"

**解决方案**:
- 确保已运行 `download_dependencies.bat`
- 检查 `third_party/` 目录是否包含 imgui、glfw、stb

### Q3: 链接错误 "unresolved external symbol"

**解决方案**:
- 清理构建目录：删除 `build/` 文件夹
- 重新运行 CMake 配置

### Q4: 运行时提示缺少 DLL

**解决方案**:
- 使用 Release 模式构建
- 或将所需 DLL 复制到可执行文件目录

---

## 手动安装依赖（如果脚本失败）

### ImGui:
```cmd
cd third_party
git clone https://github.com/ocornut/imgui.git
```

### GLFW:
```cmd
cd third_party
git clone https://github.com/glfw/glfw.git
```

### STB:
```cmd
mkdir third_party\stb
cd third_party\stb
```

然后手动下载以下文件到 `third_party/stb/`:
- https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
- https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
- https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize2.h

---

## 开发环境推荐

### Visual Studio Code

**推荐扩展**:
- C/C++ (Microsoft)
- CMake Tools
- CMake

### Visual Studio

直接打开生成的 `.sln` 文件即可。

---

## 项目结构

```
ImageBatchTool/
├── CMakeLists.txt          # CMake 配置
├── build.bat               # Windows 构建脚本
├── download_dependencies.bat  # 依赖下载脚本
├── third_party/            # 第三方库
│   ├── imgui/
│   ├── glfw/
│   └── stb/
├── src/                    # 源代码
│   ├── main.cpp
│   ├── app/
│   ├── ui/
│   ├── core/
│   ├── task/
│   └── utils/
└── build/                  # 构建输出（自动生成）
    └── bin/
        └── Release/
            └── ImageBatchTool.exe
```

---

## 技术支持

如果遇到问题：
1. 检查是否安装了所有前置要求
2. 确保所有工具都在系统 PATH 中
3. 尝试清理构建目录后重新构建
4. 查看 CMake 和编译器的错误信息

---

## 下一步

构建成功后，请参考 [README.md](README.md) 了解如何使用本工具。
