# 项目规则与编译运行指南

**项目**: Image Batch Tool - Photoshop 选区越界粉色边框提示  
**生成日期**: 2026-01-31  
**版本**: 1.0  
**状态**: ✅ 重构完成，编译就绪

---

## 一、项目架构规则

### 1.1 模块分层原则

项目遵循**严格的分层架构**，每层各司其职：

```
应用层 (Application)
    ↓
UI 层 (Presentation)
    ├── MainUI
    ├── PreviewPanel ←── 使用 SelectionSystem, OutOfBoundsRenderer
    ├── ControlPanel
    └── SettingsPanel
    ↓
业务逻辑层 (Business Logic)
    ├── SelectionSystem (Controller) ←── 处理交互和状态
    ├── OutOfBoundsRenderer (Renderer) ←── 负责渲染
    └── TransformManager
    ↓
数学计算层 (Mathematics) ← 新增
    └── SelectionMath ←── 所有几何计算集中在这里
    ↓
工具层 (Utilities)
    ├── ImageLoader
    ├── ImageProcessor
    ├── GuideLineManager
    └── SelectionHistory
    ↓
数据结构层 (Data Structures)
    └── Types.h (SelectionRect, LayerPixelBounds, 等)
```

### 1.2 职责分离规则

| 模块 | 职责 | 禁止 |
|------|------|------|
| **SelectionMath** | 几何计算<br>坐标转换 | 渲染<br>UI 交互<br>状态管理 |
| **SelectionSystem** | 交互处理<br>状态管理<br>历史记录 | 几何计算<br>越界判断 |
| **OutOfBoundsRenderer** | 粉色线渲染<br>色彩管理<br>线条样式 | 越界判断<br>交互处理 |
| **PreviewPanel** | UI 集成<br>渲染循环<br>坐标映射 | 业务逻辑<br>数据计算 |

### 1.3 数据流向规则

```
用户输入 (鼠标事件)
    ↓
SelectionSystem::Update()
    ├─ 处理交互状态
    ├─ 更新 SelectionRect
    └─ CommitSelection()
    ↓
PreviewPanel::Render()
    ├─ 获取选区: m_SelectionSystem.GetSelection()
    ├─ 获取图层: 从 ProcessConfig 获取
    │
    ├─ 计算越界状态:
    │   m_OutOfBoundsRenderer.Update(
    │       selectionPos, selectionSize,
    │       layerPos, layerSize
    │   )
    │   ↓ (内部调用)
    │   SelectionMath::ComputeOutOfBoundsFlags()
    │
    ├─ 渲染顺序:
    │   1. m_SelectionSystem.Render()      // 蚂蚁线
    │   2. m_OutOfBoundsRenderer.Render()  // 粉色线
    │
    └─ 呈现到屏幕
```

---

## 二、编译规则

### 2.1 编译环境要求

| 要求 | 最低版本 | 推荐版本 |
|------|---------|---------|
| C++ 标准 | C++14 | C++17 |
| CMake | 3.10 | 3.20+ |
| Visual Studio | 2019 | 2022 |
| GCC | 7.0 | 11.0+ |
| Clang | 6.0 | 14.0+ |

### 2.2 CMakeLists.txt 规则

**必须包含的源文件:**
```cmake
set(PROJECT_SOURCES
    # ... 其他文件 ...
    
    # SelectionSystem 模块
    ${CMAKE_SOURCE_DIR}/src/core/SelectionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SelectionSystem.h
    ${CMAKE_SOURCE_DIR}/src/core/SelectionHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SelectionHistory.h
    
    # SelectionMath 模块 ← 必须添加！
    ${CMAKE_SOURCE_DIR}/src/core/SelectionMath.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SelectionMath.h
    
    # OutOfBoundsRenderer 模块
    ${CMAKE_SOURCE_DIR}/src/core/OutOfBoundsRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/OutOfBoundsRenderer.h
)
```

### 2.3 编译标志规则

**Windows + MSVC:**
```cmake
if(WIN32 AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
endif()
```

**Linux + GCC:**
```cmake
if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -fPIC")
endif()
```

### 2.4 静态链接规则

**目标**: 生成独立的 EXE 文件，无外部依赖

```cmake
# 所有库都静态链接
option(BUILD_SHARED_LIBS OFF)
option(GLFW_BUILD_SHARED_LIBS OFF)

# 运行时库静态链接 (Windows)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")  # Release
endif()
```

---

## 三、编译命令

### 3.1 Windows + Visual Studio 2022

```bash
# 1. 进入项目目录
cd f:\oneDrive\Desktop\ps-cpp

# 2. 创建 build 目录
mkdir build_release
cd build_release

# 3. 配置 CMake
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release ..

# 4. 编译
cmake --build . --config Release --parallel 4

# 5. 运行
.\bin\Release\ImageBatchTool.exe
```

**预期输出:**
```
Build files have been written to: ...
[100%] Built target ImageBatchTool
```

### 3.2 Windows + MSVC (命令行)

```bash
# 1. 设置 MSVC 环境
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

# 2. 进入项目目录
cd f:\oneDrive\Desktop\ps-cpp

# 3. 创建 build 目录
mkdir build_nmake
cd build_nmake

# 4. 配置 CMake
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl ..

# 5. 编译
nmake

# 6. 运行
.\bin\Release\ImageBatchTool.exe
```

### 3.3 Linux / macOS

```bash
# 1. 进入项目目录
cd ~/projects/ps-cpp  # 或你的项目路径

# 2. 创建 build 目录
mkdir build_cmake
cd build_cmake

# 3. 配置 CMake
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..

# 4. 编译
make -j4

# 5. 运行
./bin/ImageBatchTool
```

### 3.4 快速清理和重新编译

```bash
# 完全清理
cd f:\oneDrive\Desktop\ps-cpp
rmdir /s /q build_release
rmdir /s /q build_cmake

# 重新编译
mkdir build_release
cd build_release
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

---

## 四、运行规则

### 4.1 程序启动检查清单

运行程序后，应该看到以下画面：

- ✅ 窗口标题: "ImageBatchTool"
- ✅ 主工作区: 显示空的画布（灰色背景）
- ✅ 左侧面板: 图像列表（初始为空）
- ✅ 右侧面板: 控制选项和设置
- ✅ 顶部菜单: 文件、编辑、视图等

### 4.2 功能测试步骤

#### 基础选区功能

1. **加载图像**
   - 菜单 → 文件 → 打开
   - 选择一张图片 (JPG/PNG)
   - 图片应显示在预览区

2. **创建选区**
   - 在图片上拖拽鼠标 (左上到右下)
   - 应看到"蚂蚁线"（黑白虚线框）
   - 选区外显示半透明黑色遮罩

3. **移动选区**
   - 在蚂蚁线内拖拽
   - 整个选区应跟随鼠标移动

4. **修饰键**
   - Shift: 创建正方形选区
   - Alt: 从中心向外创建
   - Shift+Alt: 中心对称正方形

#### 越界提示功能 ✅ **关键测试**

1. **向左越界**
   - 创建选区，拖拽左边超过图片左边界
   - 应看到 **粉色竖线** 在图片左边界处
   - 其他边不显示粉色线

2. **向右越界**
   - 拖拽选区右边超过图片右边界
   - 应看到 **粉色竖线** 在图片右边界处

3. **向上越界**
   - 拖拽选区上边超过图片上边界
   - 应看到 **粉色横线** 在图片上边界处

4. **向下越界**
   - 拖拽选区下边超过图片下边界
   - 应看到 **粉色横线** 在图片下边界处

5. **多边越界**
   - 拖拽选区，使其同时超过左边和上边界
   - 应看到 **粉色L形线条**（左竖线 + 上横线）

6. **粉色线层级**
   - 粉色线应 **清晰可见**，在蚂蚁线上方
   - 蚂蚁线不应覆盖粉色线

7. **实时更新**
   - 快速拖拽选区
   - 粉色线应 **实时显示/隐藏**，不闪烁

8. **清除选区**
   - 按 Ctrl+D 或菜单 → 选择 → 无
   - 粉色线应立即 **消失**

### 4.3 性能基准

| 测试场景 | 预期指标 |
|---------|---------|
| 选区创建 | < 1ms |
| 越界判断 | < 0.5ms |
| 粉色线渲染 | < 0.2ms |
| 帧率 (60 FPS) | ≥ 50 FPS |
| 内存占用 | < 100MB |

---

## 五、代码审查规则

### 5.1 SelectionMath 代码审查

审查清单：

```cpp
// ✅ 必须有完整的文档注释
/// @brief 计算选区与图层边界的交集
/// @param selection 选区（标准化）
/// @param layerBounds 图层像素边界
/// @return 交集矩形

// ✅ 必须有输入验证
if (!selection.IsValid() || !layerBounds.IsValid()) {
    return IntersectionRect(); // 返回空矩形
}

// ✅ 必须处理浮点数精度
const float epsilon = 1e-6f;
if (std::abs(value1 - value2) < epsilon) { ... }

// ✅ 必须考虑边界情况
// e.g., 选区完全在图层外
```

### 5.2 SelectionSystem 代码审查

```cpp
// ✅ 不能包含越界逻辑
// ❌ 错误: 在 SelectionSystem 中判断越界
if (selection.x < layer.x) { /* 越界 */ }

// ✅ 正确: 调用 SelectionMath
auto flags = SelectionMath::ComputeOutOfBoundsFlags(sel, bounds);
```

### 5.3 OutOfBoundsRenderer 代码审查

```cpp
// ✅ 必须使用 SelectionMath 计算
m_OutOfBoundsFlags = SelectionMath::ComputeOutOfBoundsFlags(...);

// ✅ 必须检查标志再渲染
if (m_OutOfBoundsFlags.HasAnyOutOfBounds()) {
    Render(...);
}

// ❌ 错误: 直接绘制整圈粉线
drawList->AddRect(...);  // 不允许！只能在越界边绘制
```

---

## 六、调试规则

### 6.1 禁止事项

❌ **严禁在代码中保留调试输出:**
```cpp
// 不允许！
printf("DEBUG: Selection = ...\n");
std::cout << "Values: " << x << ", " << y << std::endl;
```

✅ **调试方法:**
```cpp
// 方法1: 使用调试器 (F5)
// 方法2: 添加临时日志到文件
if (debugMode) {
    logger.Debug("SelectionMath::ComputeOutOfBoundsFlags called");
}
// 方法3: 使用 assert 进行单元测试
assert(intersection.width >= 0);
```

### 6.2 调试步骤

1. **编译调试版本**
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   cmake --build . --config Debug
   ```

2. **使用 Visual Studio 调试器**
   - F5: 启动调试
   - F10: 步过
   - F11: 步入
   - Shift+F11: 步出

3. **观察数据**
   - 在 Update() 中设置断点
   - 检查 SelectionRect 的值
   - 检查 OutOfBoundsFlags 的状态

4. **追踪坐标转换**
   - 检查画布逻辑坐标是否正确
   - 检查屏幕坐标转换是否准确

---

## 七、版本控制规则

### 7.1 Commit 消息规范

```
[功能] 简短描述 (最多50字符)

详细说明（可选）：
- 修改了哪些文件
- 为什么要这样修改
- 影响范围

示例:
[重构] SelectionMath 模块独立几何计算

- 新增 SelectionMath.h/cpp 数学库
- 将越界判断从 OutOfBoundsRenderer 移出
- OutOfBoundsRenderer 现在仅负责渲染
- 增强代码复用性和可维护性

相关文件:
- src/core/SelectionMath.h/cpp (新增)
- src/core/OutOfBoundsRenderer.h/cpp (重构)
- CMakeLists.txt (更新)
```

### 7.2 分支管理

```
main
  ├── develop
  │    ├── feature/selection-math
  │    ├── feature/out-of-bounds-renderer
  │    └── hotfix/bug-fix
  └── release/v1.0
```

### 7.3 Tag 命名

```
v1.0.0          // 主要版本
v1.0.1          // 修复版本
v1.1.0          // 功能版本
feature/selection-improvement
```

---

## 八、测试规则

### 8.1 单元测试框架

推荐使用 Google Test (gtest):

```cpp
#include <gtest/gtest.h>
#include "core/SelectionMath.h"

TEST(SelectionMathTest, ComputeIntersection) {
    // Arrange
    auto selection = SelectionMath::CreateSelectionRect(0, 0, 100, 100);
    auto bounds = SelectionMath::CreateLayerBounds(50, 50, 100, 100);
    
    // Act
    auto intersection = SelectionMath::ComputeIntersection(selection, bounds);
    
    // Assert
    EXPECT_EQ(intersection.x, 50);
    EXPECT_EQ(intersection.y, 50);
    EXPECT_EQ(intersection.width, 50);
    EXPECT_EQ(intersection.height, 50);
}

TEST(SelectionMathTest, ComputeOutOfBoundsFlags_LeftOverflow) {
    auto selection = SelectionMath::CreateSelectionRect(-10, 0, 100, 100);
    auto bounds = SelectionMath::CreateLayerBounds(0, 0, 100, 100);
    
    auto flags = SelectionMath::ComputeOutOfBoundsFlags(selection, bounds);
    
    EXPECT_TRUE(flags.left);
    EXPECT_FALSE(flags.right);
    EXPECT_FALSE(flags.top);
    EXPECT_FALSE(flags.bottom);
}
```

### 8.2 集成测试

```cpp
// 在 PreviewPanel 中测试完整流程
TEST(PreviewPanelTest, SelectionOutOfBoundsVisualization) {
    PreviewPanel panel;
    
    // 设置 100x100 图层
    ImageLayer layer = CreateTestLayer(100, 100);
    
    // 创建 -10 到 110 的选区 (超出边界)
    m_SelectionSystem.SetSelection(-10, -10, 120, 120);
    
    // 渲染一帧
    panel.Render(...);
    
    // 验证粉色线渲染标志
    EXPECT_TRUE(m_OutOfBoundsRenderer.GetOutOfBoundsFlags().left);
    EXPECT_TRUE(m_OutOfBoundsRenderer.GetOutOfBoundsFlags().right);
    EXPECT_TRUE(m_OutOfBoundsRenderer.GetOutOfBoundsFlags().top);
    EXPECT_TRUE(m_OutOfBoundsRenderer.GetOutOfBoundsFlags().bottom);
}
```

---

## 九、性能优化规则

### 9.1 优化目标

| 指标 | 目标值 | 方法 |
|------|-------|------|
| 单帧时间 | < 16.67ms (60 FPS) | 缓存几何数据 |
| 内存占用 | < 100MB | 复用对象，避免频繁分配 |
| 启动时间 | < 2s | 懒加载资源 |

### 9.2 缓存策略

```cpp
// ✅ 正确: 缓存计算结果
class OutOfBoundsRenderer {
private:
    SelectionMath::OutOfBoundsFlags m_OutOfBoundsFlags;  // 缓存
    SelectionMath::SelectionRect m_CachedSelection;      // 缓存
};

// 仅当输入改变时才重新计算
if (m_CachedSelection != newSelection) {
    m_OutOfBoundsFlags = SelectionMath::ComputeOutOfBoundsFlags(...);
    m_CachedSelection = newSelection;
}
```

### 9.3 避免的操作

❌ **性能杀手:**
- 频繁的内存分配 (new/delete)
- 深拷贝大数据结构
- 嵌套的 for 循环遍历几何数据
- 重复的计算不缓存结果

✅ **性能优化:**
- 使用栈分配而不是堆分配
- 传递引用而不是值
- 缓存计算结果
- 早期返回避免不必要计算

---

## 十、常见问题与解决方案

### Q1: 编译错误 "SelectionMath.h: No such file or directory"

**原因**: CMakeLists.txt 中未添加 SelectionMath  
**解决方案**:
```cmake
# 在 PROJECT_SOURCES 中添加:
${CMAKE_SOURCE_DIR}/src/core/SelectionMath.cpp
${CMAKE_SOURCE_DIR}/src/core/SelectionMath.h
```

### Q2: 链接错误 "undefined reference to SelectionMath::..."

**原因**: SelectionMath.cpp 编译未包含在链接中  
**解决方案**: 重新运行 CMake 配置和清理构建

```bash
rm -rf build_release
mkdir build_release
cd build_release
cmake ..
cmake --build .
```

### Q3: 粉色线不显示

**原因**: 几种可能
1. OutOfBoundsRenderer::Update() 未被调用
2. 选区未超出图层边界
3. 图层 Bounds 设置错误

**调试步骤**:
```cpp
// 在 OutOfBoundsRenderer::Update() 中添加断点
// 检查:
// 1. 输入参数是否正确
// 2. ComputeOutOfBoundsFlags() 返回值
// 3. m_OutOfBoundsFlags.HasAnyOutOfBounds() 是否为 true
```

### Q4: 编译警告 C4996

**原因**: 使用了已弃用的 API  
**解决方案**:
```cpp
#define _CRT_SECURE_NO_WARNINGS  // 在 Types.h 顶部
```

### Q5: 程序运行缓慢

**原因**: 调试模式或无优化  
**解决方案**:
```bash
# 使用 Release 模式编译
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

---

## 十一、检查清单

### 代码完成清单

- [x] SelectionMath 已创建
- [x] SelectionSystem 已重构
- [x] OutOfBoundsRenderer 已重构
- [x] PreviewPanel 已更新
- [x] CMakeLists.txt 已更新
- [x] 调试输出已清理
- [x] 注释文档完整
- [x] 规范文档编写

### 编译运行清单

- [ ] 编译无错误
- [ ] 编译无警告
- [ ] 程序启动正常
- [ ] UI 界面显示正确
- [ ] 所有菜单可用

### 功能测试清单

- [ ] 选区创建功能正常
- [ ] 选区移动功能正常
- [ ] 选区越界显示粉色线
- [ ] 粉色线仅在越界边显示
- [ ] 粉色线实时更新
- [ ] Undo/Redo 功能正常

### 最终检查清单

- [ ] 代码审查通过
- [ ] 所有测试通过
- [ ] 性能满足要求
- [ ] 文档完整准确
- [ ] 可以提交版本控制

---

## 总结

本文档规定了 Image Batch Tool 项目的开发标准和最佳实践。

**核心原则:**
1. **分层架构**: 严格的模块分离
2. **单一职责**: 每个模块只做一件事
3. **代码质量**: 生产级别标准
4. **文档优先**: 清晰的文档和注释
5. **规范遵守**: 完全遵循 Photoshop 设计

**遵循本规则，确保项目**
- 代码可维护性强
- 功能可靠稳定
- 性能优异高效
- 扩展灵活易行

---

**版本**: 1.0  
**更新日期**: 2026-01-31  
**维护人**: AI Assistant  
**状态**: ✅ 有效
