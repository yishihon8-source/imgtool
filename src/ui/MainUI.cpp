#include "MainUI.h"
#include "ImageListPanel.h"
#include "PreviewPanel.h"
#include "ControlPanel.h"
#include "SettingsPanel.h"
#include "utils/FileDialog.h"
#include "core/ImageLoader.h"
#include "utils/Logger.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstdio>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

MainUI::MainUI() {
    m_ImageListPanel = std::make_unique<ImageListPanel>();
    m_PreviewPanel = std::make_unique<PreviewPanel>();
    m_ControlPanel = std::make_unique<ControlPanel>();
    m_SettingsPanel = std::make_unique<SettingsPanel>();
    m_BatchProcessor = std::make_unique<BatchProcessor>();

    // 设置默认画布参数为 750x1000，背景白色
    m_ProcessConfig.canvas = Canvas(750, 1000, Color::White());
    m_TempCanvas = Canvas(750, 1000, Color::White());
    
    // 设置默认输出格式为 JPG
    m_ProcessConfig.format = OutputFormat::JPG;
    m_ProcessConfig.jpgQuality = 95;
    
    SetupProfessionalStyle();
}

MainUI::~MainUI() = default;

void MainUI::SetupProfessionalStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // ===== 原型图配色方案 =====
    ImVec4* colors = style.Colors;
    
    // 主背景 - 深灰色 (#1E1E1E)
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    
    // 边框
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // 标题栏
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    
    // 文本
    colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    
    // 强调色 - 蓝色
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.55f);
    
    // 按钮
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    
    // 输入框
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    
    // 滑块
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);
    
    // 选中状态
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    
    // Tab
    colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    
    // Docking
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    
    // ===== 间距与圆角 =====
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    
    // 圆角
    style.WindowRounding = 0.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    // 边框
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;
}

void MainUI::Render() {
    // 更新通知计时器（仅用于导入操作的提示框）
    if (m_ShowNotification && m_NotificationType == NotificationType::Success) {
        m_NotificationTimer += ImGui::GetIO().DeltaTime;
        Logger::Debug("Notification timer: " + std::to_string(m_NotificationTimer) + "s");
        
        // 5秒后自动关闭通知（仅对成功消息，且用户没有手动关闭的情况）
        if (m_NotificationTimer >= 5.0f) {
            Logger::Info("Success notification timer expired, closing notification");
            m_ShowNotification = false;
            m_NotificationTimer = 0.0f;
        }
    }
    
    RenderTopBar();
    SetupDockSpace();

    // 渲染三大面板
    m_ImageListPanel->Render(m_ImageList, m_CurrentImageIndex,
        [this]() { AddImages(); },
        [this]() { AddImagesFromFolder(); },
        [this]() { ClearAllImages(); }
    );
    
    // 传递工具状态给 PreviewPanel（使用引用以便双向同步）
    bool transformMode = (m_CurrentTool == ToolMode::Transform);
    bool selectionMode = (m_CurrentTool == ToolMode::Selection);
    
    m_PreviewPanel->Render(m_ImageList, m_CurrentImageIndex, m_ProcessConfig, m_CanvasApplied, 
                           transformMode, selectionMode);
    
    // 同步回工具状态
    if (transformMode && !selectionMode) {
        m_CurrentTool = ToolMode::Transform;
    } else if (selectionMode && !transformMode) {
        m_CurrentTool = ToolMode::Selection;
    } else if (!transformMode && !selectionMode) {
        m_CurrentTool = ToolMode::None;
    }
    
    // 渲染左侧工具栏（在 PreviewPanel 之后，这样能立即反映状态变化）
    RenderToolbar();
    
    // ControlPanel - 内联实现
    RenderControlPanel(m_ProcessConfig);

    // 底部状态栏已移除
    // RenderBottomStatusBar();

    // 关于对话框
    if (m_ShowAbout) {
        RenderAboutDialog();
    }
    
    // 设置面板
    if (m_ShowSettings) {
        m_SettingsPanel->Render(m_ShowSettings);
    }
    
    // 渲染导入提示对话框
    RenderNotificationDialog();
    
    // 渲染处理完成对话框
    RenderBatchProcessCompleteDialog();
}

void MainUI::RenderTopBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 120));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 0));  // 上下padding设为0，手动控制
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(16, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));
    
    ImGui::Begin("##TopBar", nullptr, flags);

    // 计算垂直居中位置（窗口总高度 - 元素高度）/ 2
    const float windowHeight = 120.0f;
    
    // Logo 按钮 (60x60)
    const float logoHeight = 60.0f;
    const float logoY = (windowHeight - logoHeight) * 0.5f;
    
    ImGui::SetCursorPosY(logoY);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.8f);
    ImGui::Button("P", ImVec2(60, 60));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor(2);
    
    // 标题文字（计算文字高度并居中）
    ImGui::SameLine();
    ImGui::SetWindowFontScale(1.4f);
    float textHeight = ImGui::GetTextLineHeight();
    ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);
    ImGui::Text("Batch Processor Pro");
    ImGui::SetWindowFontScale(1.0f);
    
    // 添加文件夹按钮 (50 高)
    ImGui::SameLine();
    const float buttonHeight = 50.0f;
    const float buttonY = (windowHeight - buttonHeight) * 0.5f;
    ImGui::SetCursorPosY(buttonY);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
    ImGui::SetWindowFontScale(1.1f);
    if (ImGui::Button("添加文件夹", ImVec2(120, 50))) {
        AddImagesFromFolder();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor(2);
    
    // 添加图片按钮 (50 高)
    ImGui::SameLine();
    ImGui::SetCursorPosY(buttonY);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
    ImGui::SetWindowFontScale(1.1f);
    if (ImGui::Button("添加图片", ImVec2(110, 50))) {
        AddImages();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor(2);

    // 右侧：状态 + 按钮
    ImGui::SameLine(viewport->WorkSize.x - 380);
    ImGui::SetCursorPosY(buttonY);  // 先设置到按钮的Y位置
    
    // 状态文字 - 使用按钮样式但禁用交互，确保高度一致
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));  // 透明背景
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Button("● 就绪", ImVec2(80, 50));  // 使用按钮确保高度一致
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor(4);
    
    // 全局设置按钮
    ImGui::SameLine();
    ImGui::SetCursorPosY(buttonY);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
    ImGui::SetWindowFontScale(1.1f);
    if (ImGui::Button("全局设置", ImVec2(100, 50))) {
        m_ShowSettings = true;
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor(2);
    
    // 开始处理按钮
    ImGui::SameLine();
    ImGui::SetCursorPosY(buttonY);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    bool canProcess = !m_ImageList.empty() && !m_BatchProcessor->IsRunning();
    ImGui::BeginDisabled(!canProcess);
    ImGui::SetWindowFontScale(1.1f);
    if (ImGui::Button("开始处理", ImVec2(110, 50))) {
        StartBatchProcess();
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::EndDisabled();
    
    ImGui::PopStyleColor(3);

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void MainUI::SetupDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // 计算工作区域
    const float topBarHeight = 120.0f;
    const float statusBarHeight = 0.0f;  // 状态栏已移除，设置为 0
    const float toolbarWidth = 80.0f;  // 从 60 更新到 80
    
    // DockSpace 从工具栏右侧开始
    ImVec2 workPos = ImVec2(viewport->WorkPos.x + toolbarWidth, viewport->WorkPos.y + topBarHeight);
    ImVec2 workSize = ImVec2(viewport->WorkSize.x - toolbarWidth, viewport->WorkSize.y - topBarHeight - statusBarHeight);
    
    ImGui::SetNextWindowPos(workPos);
    ImGui::SetNextWindowSize(workSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    
    // 首次运行时设置布局
    static bool first_time = true;
    if (first_time) {
        first_time = false;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, workSize);

        // 三栏布局：左侧素材列表 (360px) + 中央画布 + 右侧属性 (380px)
        ImGuiID dock_main_id = dockspace_id;
        
        // 左侧：素材列表 (固定 360px)
        ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 360.0f / workSize.x, nullptr, &dock_main_id);
        
        // 右侧：属性检查器 (固定 380px)
        ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 380.0f / (workSize.x - 360.0f), nullptr, &dock_main_id);

        // 停靠窗口
        ImGui::DockBuilderDockWindow("素材列表", dock_left_id);
        ImGui::DockBuilderDockWindow("画布", dock_main_id);
        ImGui::DockBuilderDockWindow("属性检查器 (Inspector)", dock_right_id);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::End();
}

void MainUI::RenderBottomStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - 28);
    ImVec2 size = ImVec2(viewport->WorkSize.x, 28);

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 4));  // 增大左右内边距
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.00f));
    
    ImGui::Begin("##StatusBar", nullptr, flags);

    // 左侧：已选中数量
    ImGui::SetWindowFontScale(1.1f);  // 状态栏字体放大
    ImGui::TextDisabled("已选中 %d 张", static_cast<int>(m_ImageList.size()));
    
    // 右侧：耗时
    ImGui::SameLine(viewport->WorkSize.x - 160);
    ImGui::TextDisabled("耗时: 2s • 0.7s");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void MainUI::RenderToolbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    const float topBarHeight = 120.0f;
    const float toolbarWidth = 80.0f;  // 从 60 增加到 80
    
    ImVec2 pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + topBarHeight);
    ImVec2 size = ImVec2(toolbarWidth, viewport->WorkSize.y - topBarHeight);  // 移除了 -28（状态栏高度）
    
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 16));  // 增大 padding
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 12));  // 增大间距
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.14f, 0.14f, 1.00f));
    
    ImGui::Begin("##Toolbar", nullptr, flags);
    
    // 变换工具按钮 - 增大尺寸和字体
    bool isTransformActive = (m_CurrentTool == ToolMode::Transform);
    if (isTransformActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    }
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);  // 增大圆角
    ImGui::SetWindowFontScale(1.4f);  // 增大字体
    if (ImGui::Button("T", ImVec2(60, 60))) {  // 从 44x44 增加到 60x60
        m_CurrentTool = (m_CurrentTool == ToolMode::Transform) ? ToolMode::None : ToolMode::Transform;
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::SetWindowFontScale(1.1f);
        ImGui::Text("变换工具 (Ctrl+T)");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::EndTooltip();
    }
    
    // 矩形选框工具按钮 - 增大尺寸和字体
    bool isSelectionActive = (m_CurrentTool == ToolMode::Selection);
    if (isSelectionActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.4f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    }
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);  // 增大圆角
    ImGui::SetWindowFontScale(1.4f);  // 增大字体
    if (ImGui::Button("M", ImVec2(60, 60))) {  // 从 44x44 增加到 60x60
        m_CurrentTool = (m_CurrentTool == ToolMode::Selection) ? ToolMode::None : ToolMode::Selection;
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::SetWindowFontScale(1.1f);
        ImGui::Text("矩形选框工具 (M)");
        ImGui::Text("支持 Shift/Alt 修饰键");
        ImGui::Text("ESC: 取消选区");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::EndTooltip();
    }
    
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

void MainUI::RenderAboutDialog() {
    ImGui::SetNextWindowSize(ImVec2(420, 280), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    ImGui::Begin("关于", &m_ShowAbout, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "Batch Processor Pro");
    ImGui::TextDisabled("版本 1.0.0");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextWrapped("专业级图片批量处理工具");
    ImGui::Spacing();
    ImGui::BulletText("画布系统（类似 Photoshop）");
    ImGui::BulletText("可视化裁剪与批量应用");
    ImGui::BulletText("多线程并行处理");
    ImGui::BulletText("支持 JPG/PNG/WEBP 格式");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextDisabled("技术栈: C++17 + ImGui + OpenGL + GLFW");
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    float buttonWidth = 100;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) * 0.5f);
    if (ImGui::Button("确定", ImVec2(buttonWidth, 32))) {
        m_ShowAbout = false;
    }
    
    ImGui::End();
}

void MainUI::StartBatchProcess() {
    if (m_ImageList.empty()) {
        ShowError("未添加任何图片!\n请先添加图片文件或文件夹");
        return;
    }

    if (!m_CanvasApplied) {
        ShowError("画布尚未应用!\n请先在右侧面板点击应用画布按钮");
        return;
    }

    // 打开输出文件夹选择对话框
    std::string outputFolder = FileDialog::OpenFolder();
    if (outputFolder.empty()) {
        return;  // 用户取消了选择
    }

    // 保存输出目录供"打开目录"使用
    m_OutputDirectory = outputFolder;

    // 按照素材列表的顺序创建任务，并传递变换状态
    std::vector<BatchTask> tasks;
    for (size_t i = 0; i < m_ImageList.size(); ++i) {
        const auto& info = m_ImageList[i];
        BatchTask task;
        task.inputPath = info.filePath;
        task.outputPath = outputFolder + "/" + info.fileName;
        task.config = m_ProcessConfig;
        task.transformState = info.transformState;  // 传递变换状态
        tasks.push_back(task);
    }

    // 使用 Lambda 捕获 this 指针
    m_BatchProcessor->Start(tasks,
        [this](const BatchProgress& progress) { 
            (void)progress; 
        },
        [this](bool success) { 
            if (success) {
                std::string message = "[OK] 批处理成功完成！\n\n处理文件数：" + 
                    std::to_string(m_ImageList.size()) + "\n" +
                    "输出目录：" + m_OutputDirectory;
                ShowBatchProcessComplete(message);
            } else {
                ShowError("批处理失败！\n请检查文件权限或重试。");
            }
        }
    );
}

void MainUI::AddImages() {
    try {
        Logger::Info("=== AddImages() started ===");
        auto files = FileDialog::OpenFiles();
        Logger::Info("FileDialog::OpenFiles() returned: " + std::to_string(files.size()) + " files");
        
        if (files.empty()) {
            Logger::Debug("No files selected, returning");
            return;
        }

        Logger::Debug("Starting to process selected files...");
        int addedCount = 0;
        int skippedCount = 0;
        int errorCount = 0;

        for (size_t i = 0; i < files.size(); ++i) {
            try {
                Logger::Debug("Processing file " + std::to_string(i + 1) + ": " + files[i]);
                ImageInfo info;
                
                Logger::Debug("Calling ImageLoader::GetInfo()...");
                if (!ImageLoader::GetInfo(files[i], info)) {
                    Logger::Error("ImageLoader::GetInfo() returned false for: " + files[i]);
                    errorCount++;
                    continue;
                }
                
                Logger::Debug("GetInfo() succeeded, checking for duplicates...");

                // 检查是否已存在
                bool exists = false;
                for (const auto& existing : m_ImageList) {
                    if (existing.filePath == info.filePath) {
                        exists = true;
                        break;
                    }
                }
                
                Logger::Debug("Duplicate check done. exists=" + std::string(exists ? "true" : "false"));
                
                if (!exists) {
                    Logger::Debug("Adding image to list...");
                    m_ImageList.push_back(info);
                    addedCount++;
                    Logger::Debug("Image added successfully");
                } else {
                    skippedCount++;
                    Logger::Debug("Image already exists, skipping");
                }
            } catch (const std::exception& e) {
                Logger::Error("Exception processing file " + std::to_string(i) + ": " + std::string(e.what()));
                errorCount++;
            } catch (...) {
                Logger::Error("Unknown exception processing file " + std::to_string(i));
                errorCount++;
            }
        }

        Logger::Info("File processing completed. Added: " + std::to_string(addedCount) + 
                     ", Skipped: " + std::to_string(skippedCount) + 
                     ", Errors: " + std::to_string(errorCount));

        if (addedCount == 0) {
            std::string message = "没有新图片被添加!";
            if (skippedCount > 0) {
                message += "\n所有 " + std::to_string(skippedCount) + " 张图片已存在于列表中";
            }
            if (errorCount > 0) {
                message += "\n(失败: " + std::to_string(errorCount) + " 张)";
            }
            Logger::Warning(message);
            ShowError(message);
        } else {
            std::string message = "成功添加 " + std::to_string(addedCount) + " 张图片";
            if (skippedCount > 0) {
                message += "\n(跳过 " + std::to_string(skippedCount) + " 张重复)";
            }
            if (errorCount > 0) {
                message += "\n(失败: " + std::to_string(errorCount) + " 张)";
            }
            Logger::Info(message);
            ShowSuccess(message);
        }

        Logger::Debug("Checking if need to auto-select first image...");
        // 如果之前没有选中，自动选中第一张
        if (m_CurrentImageIndex == -1 && !m_ImageList.empty()) {
            m_CurrentImageIndex = 0;
            Logger::Debug("Auto-selected first image");
        }
        
        Logger::Info("=== AddImages() completed successfully ===");
    } catch (const std::exception& e) {
        Logger::Error("Exception in AddImages(): " + std::string(e.what()));
        ShowError("导入图片时发生异常!\n\n" + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in AddImages()");
        ShowError("导入图片时发生未知错误!\n请重试");
    }
}

void MainUI::AddImagesFromFolder() {
    try {
        Logger::Info("=== AddImagesFromFolder() started ===");
        Logger::Debug("Calling FileDialog::OpenFolder()...");
        std::string folderPath = FileDialog::OpenFolder();
        Logger::Info("FileDialog::OpenFolder() returned: " + (folderPath.empty() ? "empty" : folderPath));
        
        if (folderPath.empty()) {
            Logger::Debug("User cancelled folder selection");
            return; // 用户取消选择
        }

        Logger::Debug("Opening folder: " + folderPath);
        std::vector<ImageInfo> folderImages;
        
        try {
            Logger::Debug("Calling ImageLoader::GetFolderImages()...");
            if (!ImageLoader::GetFolderImages(folderPath, folderImages)) {
                Logger::Error("GetFolderImages() returned false");
                ShowError("读取文件夹失败!\n\n可能的原因:\n- 文件夹权限不足\n- 路径无效或不存在\n- 磁盘读取错误");
                return;
            }
            Logger::Info("GetFolderImages() succeeded, found " + std::to_string(folderImages.size()) + " images");
        } catch (const std::exception& e) {
            Logger::Error("Exception in GetFolderImages(): " + std::string(e.what()));
            ShowError("读取文件夹异常!\n\n错误: " + std::string(e.what()));
            return;
        } catch (...) {
            Logger::Error("Unknown exception in GetFolderImages()");
            ShowError("读取文件夹发生未知错误!\n请检查文件夹是否存在或权限是否正确");
            return;
        }

        if (folderImages.empty()) {
            Logger::Warning("No images found in folder");
            ShowError("文件夹中没有找到支持的图片!\n\n支持的格式:\n- JPG / JPEG\n- PNG\n- BMP\n- TGA");
            return;
        }

        Logger::Debug("Processing " + std::to_string(folderImages.size()) + " images from folder...");
        int addedCount = 0;
        int skippedCount = 0;

        for (size_t i = 0; i < folderImages.size(); ++i) {
            try {
                Logger::Debug("Processing image " + std::to_string(i + 1) + ": " + folderImages[i].fileName);
                // 检查是否已存在
                bool exists = false;
                for (const auto& existing : m_ImageList) {
                    if (existing.filePath == folderImages[i].filePath) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    m_ImageList.push_back(folderImages[i]);
                    addedCount++;
                    Logger::Debug("Image added");
                } else {
                    skippedCount++;
                    Logger::Debug("Image already exists, skipped");
                }
            } catch (const std::exception& e) {
                Logger::Error("Error adding image " + folderImages[i].fileName + ": " + std::string(e.what()));
                skippedCount++;
                continue;
            } catch (...) {
                Logger::Error("Unknown error adding image " + folderImages[i].fileName);
                skippedCount++;
                continue;
            }
        }

        Logger::Info("Folder processing completed. Added: " + std::to_string(addedCount) + 
                     ", Skipped: " + std::to_string(skippedCount));

        if (addedCount == 0) {
            std::string message;
            if (skippedCount > 0) {
                message = "没有新图片被添加!\n\n所有 " + std::to_string(skippedCount) + " 张图片已存在于列表中";
            } else {
                message = "没有图片被添加!\n请检查文件";
            }
            Logger::Warning(message);
            ShowError(message);
        } else {
            std::string message = "成功添加 " + std::to_string(addedCount) + " 张图片";
            if (skippedCount > 0) {
                message += "\n(跳过 " + std::to_string(skippedCount) + " 张重复图片)";
            }
            Logger::Info(message);
            ShowSuccess(message);
        }

        Logger::Debug("Checking if need to auto-select first image...");
        // 如果之前没有选中，自动选中第一张
        if (m_CurrentImageIndex == -1 && !m_ImageList.empty()) {
            m_CurrentImageIndex = 0;
            Logger::Debug("Auto-selected first image");
        }
        
        Logger::Info("=== AddImagesFromFolder() completed successfully ===");
    } catch (const std::exception& e) {
        Logger::Error("Exception in AddImagesFromFolder(): " + std::string(e.what()));
        ShowError("导入文件夹时发生异常!\n\n" + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in AddImagesFromFolder()");
        ShowError("导入文件夹时发生未知错误!\n请重试");
    }
}

void MainUI::OpenFileExplorer() {
    // 打开已保存的输出目录
    if (!m_OutputDirectory.empty()) {
        FileDialog::OpenInExplorer(m_OutputDirectory);
    } else {
        // 如果还没有选择输出目录，打开我的电脑
        FileDialog::OpenInExplorer("");
    }
}

void MainUI::ClearAllImages() {
    try {
        Logger::Info("=== ClearAllImages() started ===");
        
        // 清空图片列表
        m_ImageList.clear();
        
        // 重置当前选中索引
        m_CurrentImageIndex = -1;
        
        Logger::Info("All images cleared successfully");
        Logger::Info("=== ClearAllImages() completed ===");
    } catch (const std::exception& e) {
        Logger::Error("Exception in ClearAllImages(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in ClearAllImages()");
    }
}

void MainUI::OnFilesDropped(const std::vector<std::string>& filePaths) {
    for (const auto& path : filePaths) {
        // 检查是文件还是文件夹
        DWORD attrs = GetFileAttributesA(path.c_str());
        
        if (attrs != INVALID_FILE_ATTRIBUTES) {
            if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
                // 是文件夹，加载文件夹中的所有图片
                std::vector<ImageInfo> folderImages;
                if (ImageLoader::GetFolderImages(path, folderImages)) {
                    for (const auto& info : folderImages) {
                        // 检查是否已存在
                        bool exists = false;
                        for (const auto& existing : m_ImageList) {
                            if (existing.filePath == info.filePath) {
                                exists = true;
                                break;
                            }
                        }
                        
                        if (!exists) {
                            m_ImageList.push_back(info);
                        }
                    }
                }
            } else {
                // 是文件，检查是否为图片
                if (ImageLoader::IsSupportedFormat(path)) {
                    ImageInfo info;
                    if (ImageLoader::GetInfo(path, info)) {
                        // 检查是否已存在
                        bool exists = false;
                        for (const auto& existing : m_ImageList) {
                            if (existing.filePath == info.filePath) {
                                exists = true;
                                break;
                            }
                        }
                        
                        if (!exists) {
                            m_ImageList.push_back(info);
                        }
                    }
                }
            }
        }
    }

    // 如果之前没有选中，自动选中第一张
    if (m_CurrentImageIndex == -1 && !m_ImageList.empty()) {
        m_CurrentImageIndex = 0;
    }
}


// ===== ControlPanel 内联实现 =====

void MainUI::RenderControlPanel(ProcessConfig& config) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));
    ImGui::Begin("属性检查器 (Inspector)", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar();
    // 图层尺寸 (Transform)
    RenderTransformSection(config.canvas);
    ImGui::Spacing();
    ImGui::Spacing();

    // 导出 (Export)
    RenderExportSection(config.format, config.jpgQuality);

    ImGui::End();
}

void MainUI::RenderTransformSection(Canvas& canvas) {
    // 避免未使用参数警告
    (void)canvas;
    
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.24f, 0.24f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
    
    // 增大标题字体
    ImGui::SetWindowFontScale(1.15f);
    bool isOpen = ImGui::CollapsingHeader("画布尺寸 (Canvas)", ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::SetWindowFontScale(1.0f);
    
    if (isOpen) {
        ImGui::PopStyleColor(3);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 增大标签字体
        ImGui::SetWindowFontScale(1.1f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextUnformatted("宽度 (W)");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 增大输入框高度和字体
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
        ImGui::SetWindowFontScale(1.15f);
        ImGui::SetNextItemWidth(-80);
        ImGui::InputInt("##W", &m_TempCanvas.width, 0, 0);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleVar();
        ImGui::SameLine();
        ImGui::SetWindowFontScale(1.1f);
        ImGui::TextDisabled("px");
        ImGui::SetWindowFontScale(1.0f);
        
        m_TempCanvas.width = std::max(1, std::min(m_TempCanvas.width, 8192));
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 增大标签字体
        ImGui::SetWindowFontScale(1.1f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextUnformatted("高度 (H)");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 增大输入框高度和字体
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
        ImGui::SetWindowFontScale(1.15f);
        ImGui::SetNextItemWidth(-80);
        ImGui::InputInt("##H", &m_TempCanvas.height, 0, 0);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleVar();
        ImGui::SameLine();
        ImGui::SetWindowFontScale(1.1f);
        ImGui::TextDisabled("px");
        ImGui::SetWindowFontScale(1.0f);
        
        m_TempCanvas.height = std::max(1, std::min(m_TempCanvas.height, 8192));
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 应用画布按钮 - 增大尺寸和字体
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.16f, 0.49f, 0.88f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        ImGui::SetWindowFontScale(1.15f);
        if (ImGui::Button("应用画布", ImVec2(-1, 50))) {
            // 应用画布设置
            m_ProcessConfig.canvas = m_TempCanvas;
            m_CanvasApplied = true;
        }
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::PopStyleColor(4);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 显示当前画布状态 - 增大字体
        ImGui::SetWindowFontScale(1.1f);
        if (m_CanvasApplied) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.3f, 1.0f));
            ImGui::TextUnformatted("[OK] 画布已应用");
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
            ImGui::TextUnformatted("[ ] 未应用画布");
            ImGui::PopStyleColor();
        }
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::Spacing();
        ImGui::Spacing();
    } else {
        ImGui::PopStyleColor(3);
    }
}


void MainUI::RenderExportSection(OutputFormat& format, int& jpgQuality) {
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.24f, 0.24f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
    
    // 增大标题字体
    ImGui::SetWindowFontScale(1.15f);
    bool isOpen = ImGui::CollapsingHeader("导出格式 (Export)", ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::SetWindowFontScale(1.0f);
    
    if (isOpen) {
        ImGui::PopStyleColor(3);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 增大标签字体
        ImGui::SetWindowFontScale(1.1f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextUnformatted("选择导出格式");
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        float buttonWidth = (ImGui::GetContentRegionAvail().x - 12) / 2.0f;
        
        bool isJPG = (format == OutputFormat::JPG);
        bool isPNG = (format == OutputFormat::PNG);
        
        // 高质量 JPG 按钮 - 增大尺寸和字体
        if (isJPG) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        }
        
        ImGui::SetWindowFontScale(1.1f);
        if (ImGui::Button("高质量 JPG", ImVec2(buttonWidth, 50))) {
            format = OutputFormat::JPG;
            jpgQuality = 95;  // 默认高质量
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
        
        // 高质量 PNG 按钮 - 增大尺寸和字体
        if (isPNG) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        }
        
        ImGui::SetWindowFontScale(1.1f);
        if (ImGui::Button("高质量 PNG", ImVec2(buttonWidth, 50))) {
            format = OutputFormat::PNG;
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // 打开目录按钮 - 增大尺寸和字体
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.24f, 0.24f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.14f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        
        ImGui::SetWindowFontScale(1.1f);
        if (ImGui::Button("打开输出目录", ImVec2(-1, 50))) {
            OpenFileExplorer();
        }
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::PopStyleColor(4);
        
        ImGui::Spacing();
        ImGui::Spacing();
    } else {
        ImGui::PopStyleColor(3);
    }
}

void MainUI::RenderNotificationDialog() {
    try {
        if (!m_ShowNotification) {
            return;
        }

        Logger::Debug("RenderNotificationDialog() - Rendering notification...");

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        if (!viewport) {
            Logger::Error("RenderNotificationDialog() - GetMainViewport returned nullptr");
            return;
        }
        
        Logger::Debug("RenderNotificationDialog() - Got viewport");
        
        // 导入图片提示框显示在顶部（原位置）
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f - 250, 
                                       viewport->WorkPos.y + 60), ImGuiCond_Always, ImVec2(0.5f, 0));
        
        ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | 
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

        std::string title = (m_NotificationType == NotificationType::Error) ? "错误" : "成功";
        
        Logger::Debug("RenderNotificationDialog() - Window title: " + title);
        Logger::Debug("RenderNotificationDialog() - Message length: " + std::to_string(m_NotificationMessage.length()));
        
        if (m_NotificationType == NotificationType::Error) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5f, 0.1f, 0.1f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.8f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.4f, 0.1f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 1.0f, 0.8f, 1.0f));
        }

        Logger::Debug("RenderNotificationDialog() - Colors pushed, calling ImGui::Begin()...");
        
        if (ImGui::Begin(title.c_str(), nullptr, flags)) {
            Logger::Debug("RenderNotificationDialog() - ImGui::Begin() succeeded");
            
            ImGui::SetWindowFontScale(1.2f);
            Logger::Debug("RenderNotificationDialog() - About to render text...");
            ImGui::TextWrapped("%s", m_NotificationMessage.c_str());
            Logger::Debug("RenderNotificationDialog() - Text rendered");
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
            
            if (ImGui::Button("关闭", ImVec2(200, 40))) {
                Logger::Debug("RenderNotificationDialog() - Close button clicked");
                m_ShowNotification = false;
                m_NotificationTimer = 0.0f;
            }
            
            ImGui::PopStyleColor(2);
            ImGui::End();
            Logger::Debug("RenderNotificationDialog() - ImGui::End() called");
        } else {
            Logger::Error("RenderNotificationDialog() - ImGui::Begin() returned false");
        }

        ImGui::PopStyleColor(2);
        Logger::Debug("RenderNotificationDialog() - Rendering complete");
    } catch (const std::exception& e) {
        Logger::Error("Exception in RenderNotificationDialog(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in RenderNotificationDialog()");
    }
}

void MainUI::ShowError(const std::string& message) {
    try {
        Logger::Debug("ShowError() called with message length: " + std::to_string(message.length()));
        m_NotificationType = NotificationType::Error;
        m_NotificationMessage = message;
        m_ShowNotification = true;
        m_NotificationTimer = 0.0f;
        Logger::Debug("ShowError() - Notification set, calling PlaySystemSound()...");
        PlaySystemSound(false);
        Logger::Debug("ShowError() - PlaySystemSound() returned");
    } catch (const std::exception& e) {
        Logger::Error("Exception in ShowError(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in ShowError()");
    }
}

void MainUI::ShowSuccess(const std::string& message) {
    try {
        Logger::Debug("ShowSuccess() called with message length: " + std::to_string(message.length()));
        m_NotificationType = NotificationType::Success;
        m_NotificationMessage = message;
        m_ShowNotification = true;
        m_NotificationTimer = 0.0f;
        Logger::Debug("ShowSuccess() - Notification set, calling PlaySystemSound()...");
        PlaySystemSound(true);
        Logger::Debug("ShowSuccess() - PlaySystemSound() returned");
    } catch (const std::exception& e) {
        Logger::Error("Exception in ShowSuccess(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in ShowSuccess()");
    }
}

void MainUI::PlaySystemSound(bool isSuccess) {
    try {
        Logger::Debug("PlaySystemSound() called, isSuccess=" + std::string(isSuccess ? "true" : "false"));
        #ifdef _WIN32
        if (isSuccess) {
            Logger::Info("Playing success sound using MessageBeep(MB_ICONEXCLAMATION)...");
            // 使用 Windows 系统提示音 - Exclamation 是常见的成功提示音
            MessageBeep(MB_ICONEXCLAMATION);
            Logger::Info("Success sound played");
        } else {
            Logger::Debug("Playing error sound using MessageBeep(MB_ICONERROR)...");
            MessageBeep(MB_ICONERROR);
            Logger::Debug("Error sound played");
        }
        #else
        Logger::Debug("PlaySystemSound() - Not on Windows, skipping sound");
        #endif
    } catch (const std::exception& e) {
        Logger::Error("Exception in PlaySystemSound(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in PlaySystemSound()");
    }
}

void MainUI::ShowBatchProcessComplete(const std::string& message) {
    try {
        Logger::Info("ShowBatchProcessComplete() called with message: " + message);
        m_ShowBatchProcessComplete = true;
        m_BatchProcessMessage = message;
        PlaySystemSound(true);
        Logger::Debug("Batch process complete dialog shown");
    } catch (const std::exception& e) {
        Logger::Error("Exception in ShowBatchProcessComplete(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in ShowBatchProcessComplete()");
    }
}

void MainUI::RenderBatchProcessCompleteDialog() {
    try {
        if (!m_ShowBatchProcessComplete) return;

        // 第一次显示时打开 popup
        if (!m_BatchCompletePopupOpened) {
            ImGui::OpenPopup("批量处理完成##Complete");
            m_BatchCompletePopupOpened = true;
            Logger::Debug("Opening batch process complete dialog popup");
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("批量处理完成##Complete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            // 显示成功标志
            ImGui::TextColored(ImVec4(0.1f, 0.8f, 0.1f, 1.0f), "[OK] 处理完成");
            ImGui::Separator();

            // 显示处理信息
            ImGui::TextWrapped("%s", m_BatchProcessMessage.c_str());
            ImGui::Spacing();
            ImGui::Spacing();

            // 完成按钮
            ImVec2 buttonSize(100, 40);
            float buttonX = (ImGui::GetWindowWidth() - buttonSize.x) / 2;
            ImGui::SetCursorPosX(buttonX);

            if (ImGui::Button("完成", buttonSize)) {
                m_ShowBatchProcessComplete = false;
                m_BatchCompletePopupOpened = false;
                ImGui::CloseCurrentPopup();
                Logger::Debug("Batch process complete dialog closed by user");
            }

            ImGui::EndPopup();
        }
    } catch (const std::exception& e) {
        Logger::Error("Exception in RenderBatchProcessCompleteDialog(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in RenderBatchProcessCompleteDialog()");
    }
}

