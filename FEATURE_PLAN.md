# PS 功能实现计划

## 功能 1: Ctrl+T 图片变换（自由变换）

### 实现思路
1. 按下 Ctrl+T 进入变换模式
2. 在图片四周显示 8 个控制点（4个角 + 4个边中点）
3. 拖拽控制点可以：
   - 角点：缩放 + 旋转
   - 边中点：单向缩放
4. 拖拽图片中心：移动位置
5. 按 Enter 确认，ESC 取消

### 需要添加的状态
```cpp
// 变换模式
bool m_TransformMode = false;
ImVec2 m_TransformScale = ImVec2(1.0f, 1.0f);
float m_TransformRotation = 0.0f;
ImVec2 m_TransformPosition = ImVec2(0.0f, 0.0f);
```

---

## 功能 2: 框选 + Backspace 擦除区域

### 实现思路
1. 鼠标拖拽框选区域（矩形选框）
2. 按 Backspace 将选中区域填充为透明色
3. 需要将图片转换为 RGBA 格式（支持透明）

### 需要添加的状态
```cpp
// 选区状态
bool m_SelectionMode = false;
bool m_IsSelecting = false;
ImVec2 m_SelectionStart;
ImVec2 m_SelectionEnd;
Rect m_SelectionRect;
```

---

## 实现优先级

由于这两个功能都比较复杂，建议分步实现：

### 第一阶段（简化版）
1. **框选擦除功能**（相对简单）
   - 鼠标拖拽框选
   - Backspace 擦除为白色/透明
   
### 第二阶段（完整版）
2. **Ctrl+T 变换功能**
   - 显示变换控制框
   - 支持缩放、旋转、移动

---

## 技术要点

### 1. 键盘快捷键检测
```cpp
// Ctrl+T
if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_T)) {
    m_TransformMode = !m_TransformMode;
}

// Backspace
if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
    EraseSelection();
}
```

### 2. 图片编辑
- 需要修改 ImageData 的像素数据
- 擦除操作需要支持透明通道（RGBA）
- 变换操作需要重新采样像素

### 3. 坐标转换
- 屏幕坐标 → 画布坐标 → 图片像素坐标
- 需要考虑缩放和偏移

---

## 建议

考虑到实现复杂度，我建议：

1. **先实现框选擦除功能**（更实用，更简单）
2. **Ctrl+T 变换功能可以简化为**：
   - 只支持缩放（不支持旋转）
   - 或者使用滑块控制而不是拖拽控制点

你觉得这个方案如何？我可以先实现框选擦除功能，然后再考虑变换功能。
