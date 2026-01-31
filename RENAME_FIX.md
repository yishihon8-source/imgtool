# Photoshop 风格矩形选区 Delete / Backspace 行为设计文档

## 1. 功能目标

实现与 Photoshop 一致的选区删除行为：

- 用户使用【矩形选区工具】选中区域
- 按下 `Delete` 或 `Backspace`
- 根据当前图层类型与状态，产生不同结果
- 删除行为 **仅作用于选区范围内**
- 不影响选区外像素

---

## 2. Delete / Backspace 的统一定义

在 Photoshop 中：

- `Delete` 与 `Backspace`
- 在矩形选区存在时
- **行为完全一致**

本文档中统一称为：**Delete 操作**

---

## 3. Delete 行为的核心规则（总览）

Delete 的最终效果，取决于三个条件：

1. 是否存在有效选区
2. 当前图层类型
3. 图层是否允许透明

核心结论一句话版：

> **能透明 → 变透明  
> 不能透明 → 填充背景色**

---

## 4. 行为前置条件判断

### 4.1 是否存在有效选区

```text
if (selection == null or selection.area == 0)
PS 行为：不执行任何操作

不删除整层

不弹窗

不报错

4.2 选区与图层关系
Delete 只作用于选区 ∩ 图层区域

超出图层部分自动忽略

不会修改图层尺寸

5. 不同图层类型的 Delete 行为
5.1 普通图层（支持透明）
条件
非背景层

未锁定透明像素

支持 Alpha 通道

Delete 行为
选区内像素 → Alpha = 0（完全透明）
视觉效果
出现棋盘格

图层内容被“挖空”

下方图层可见

这是 PS 中最常见的 Delete 行为
5.2 背景层（不支持透明）
条件
图层类型为 Background

或透明通道被禁用

Delete 行为
选区内像素 → 填充当前背景色
默认：

白色（RGB 255,255,255）

PS 特点
不会变透明

不会挖空

始终保持不透明

5.3 普通图层 +「锁定透明像素」
条件
图层支持透明

但启用了「Lock Transparency」

Delete 行为
不产生任何变化
PS 行为
Delete 被忽略

不提示错误

不修改像素

6. 选区删除的精确作用范围
6.1 实际删除区域计算
DeleteArea = Selection ∩ LayerRect
Delete 永远不会超出图层边界

Delete 不会改变图层尺寸

Delete 不会影响选区外内容

7. Delete 像素级实现规范
7.1 普通图层（可透明）
for pixel in DeleteArea:
    pixel.rgba = (0, 0, 0, 0)
说明：

RGB 值可清零或保留

Alpha 必须为 0

7.2 背景层（不可透明）
for pixel in DeleteArea:
    pixel.rgb = backgroundColor
    pixel.alpha = 1.0
7.3 锁定透明像素
// 不执行任何写操作
8. 选区与 Delete 的交互细节（PS 级）
8.1 Delete 不会清除选区
Delete 后选区仍然存在

用户可继续：

移动选区

填充

反选

再次 Delete

8.2 多次 Delete 行为
第一次 Delete → 清空像素

再次 Delete → 无变化

不产生累计副作用

8.3 Delete 与 Undo
Delete 是一次 完整的可撤销操作

Undo 恢复：

像素数据

Alpha 通道

不恢复选区状态（PS 默认）

9. 推荐状态流程（实现用）
KeyDown(Delete)
 ├─ 无选区 → return
 ├─ 图层锁定 → return
 ├─ 计算 DeleteArea
 ├─ 判断图层类型
 ├─ 执行像素修改
 └─ Push Undo Record
10. 常见错误实现（❌）
Delete 时删除整层

Delete 后清空选区

Delete 改变图层尺寸

对背景层错误设置透明

锁定透明像素仍可删除

这些都 不符合 Photoshop 行为

11. 设计总结
Photoshop 的 Delete
本质是一次 受选区约束的像素修改操作

不是：

图层操作

选区操作

几何裁剪

而是：

在选区内，按图层规则修改像素

12. 可扩展方向（后续）
该设计可自然扩展到：

Edit → Clear

填充（Fill）

内容识别填充

橡皮擦工具

快速蒙版删除