# PS 风格选区自动收缩到图层范围（Selection Clamp）设计文档

## 1. 功能说明

实现与 Photoshop 一致的矩形选区行为：

- 用户可自由拖拽鼠标创建选区
- 拖拽范围允许超过图层边界
- **最终显示的选区永远不会超出图层有效区域**
- 当选区超过图层时，自动缩减到图层大小
- 不限制用户操作，仅限制选区结果

核心定义：

> 最终选区 = 用户拖拽选区 ∩ 图层区域

---

## 2. 设计原则

### 2.1 不限制用户拖拽行为
- 不在鼠标事件阶段判断是否越界
- 鼠标可以拖到任意位置
- 不提前阻断拖拽或回弹鼠标

### 2.2 强制裁剪最终选区
- 所有边界逻辑只存在于 **选区计算阶段**
- 绘制阶段只使用裁剪后的选区
- 原始拖拽矩形不直接参与绘制

---

## 3. 坐标与数据定义

### 3.1 图层矩形（Layer Rect）

```text
layerLeft
layerTop
layerRight  = layerLeft + layerWidth
layerBottom = layerTop  + layerHeight
表示图层在画布 / 世界坐标中的真实可编辑范围。

3.2 原始选区矩形（Raw Selection）
由鼠标按下点与当前点计算得到，可能越界。

rawLeft   = min(start.x, current.x)
rawTop    = min(start.y, current.y)
rawRight  = max(start.x, current.x)
rawBottom = max(start.y, current.y)
4. 核心算法：选区自动收缩（矩形交集）
4.1 数学模型
FinalSelection = RawSelection ∩ LayerRect
即：两个矩形的交集运算。

4.2 计算方式（核心逻辑）
finalLeft   = max(rawLeft,   layerLeft);
finalTop    = max(rawTop,    layerTop);
finalRight  = min(rawRight,  layerRight);
finalBottom = min(rawBottom, layerBottom);
约束说明：

左 / 上 / 右 / 下逻辑必须完全对称

不对右或下边界做 -1、padding 等修正

不区分拖拽方向

4.3 无效选区处理
当选区完全在图层外或无面积时：

if (finalRight <= finalLeft || finalBottom <= finalTop) {
    // 选区无效，不绘制
}
行为与 PS 保持一致：
不显示选区框。

5. 绘制规则
只允许绘制 FinalSelection

禁止绘制原始拖拽矩形

禁止在绘制阶段再进行边界判断

DrawSelectionRect(finalSelection);
6. 交互行为对照表
用户操作情况	最终选区表现
完全在图层内拖拽	正常选区
左 / 上越界	自动贴合图层左 / 上
右 / 下越界	自动贴合图层右 / 下
完全在图层外	不显示选区
继续拖拽	选区大小保持不变
7. 常见错误与规避
常见错误
在鼠标事件中限制坐标

使用 width / height 替代 right / bottom

右 / 下边界提前判定越界

int / float 混用导致提前裁剪

左右 / 上下逻辑不对称

正确做法
鼠标系统：完全自由

选区系统：只做矩形交集

绘制系统：只认最终结果

8. 推荐代码结构
Rect CalcRawSelection(Vec2 start, Vec2 current);
Rect IntersectRect(Rect a, Rect b);

Rect selection = IntersectRect(
    CalcRawSelection(dragStart, dragCurrent),
    layerRect
);
该结构可复用于：

矩形选区

裁剪工具

ROI 区域

遮罩系统

后续套索 / 椭圆选区

9. 总结
PS 风格选区的核心不是限制用户操作，
而是对最终选区结果进行强制裁剪。

只要严格遵循
“选区 = 拖拽结果 ∩ 图层区域”
即可实现稳定、专业、无边界 Bug 的选区系统。