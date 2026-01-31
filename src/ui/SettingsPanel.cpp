#include "SettingsPanel.h"
#include <imgui.h>

SettingsPanel::SettingsPanel() {
}

void SettingsPanel::Render(bool& isOpen) {
    if (!isOpen) {
        return;
    }

    // 设置面板大小（较大）
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("全局设置", &isOpen, flags)) {
        // 左右分栏布局
        ImGui::BeginChild("##SettingsLeft", ImVec2(200, 0), true);
        RenderSideMenu();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##SettingsRight", ImVec2(0, 0), true);
        RenderContent();
        ImGui::EndChild();

        ImGui::End();
    }
}

void SettingsPanel::RenderSideMenu() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));

    // 帮助菜单项
    bool isHelpSelected = (m_CurrentPage == SettingsPage::Help);
    if (isHelpSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    }

    if (ImGui::Button("帮助", ImVec2(-1, 0))) {
        m_CurrentPage = SettingsPage::Help;
    }
    ImGui::PopStyleColor(3);

    // 关于菜单项
    bool isAboutSelected = (m_CurrentPage == SettingsPage::About);
    if (isAboutSelected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    }

    if (ImGui::Button("关于", ImVec2(-1, 0))) {
        m_CurrentPage = SettingsPage::About;
    }
    ImGui::PopStyleColor(3);

    ImGui::PopStyleVar(2);
}

void SettingsPanel::RenderContent() {
    switch (m_CurrentPage) {
        case SettingsPage::Help:
            RenderHelpPage();
            break;
        case SettingsPage::About:
            RenderAboutPage();
            break;
    }
}

void SettingsPanel::RenderHelpPage() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));

    ImGui::SetWindowFontScale(1.3f);
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "快捷键指南");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    // 画布导航
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "画布导航");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::Indent(20);
    ImGui::BulletText("Alt + 鼠标滚轮");
    ImGui::Indent(20);
    ImGui::TextDisabled("以鼠标为中心缩放画布视图 (10%%-800%%)");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("Ctrl + 鼠标滚轮");
    ImGui::Indent(20);
    ImGui::TextDisabled("左右移动画布（水平滚动）");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("空格 + 拖拽");
    ImGui::Indent(20);
    ImGui::TextDisabled("抓手工具，自由移动画布（类似 Photoshop）");
    ImGui::Unindent(20);
    ImGui::Spacing();
    ImGui::Unindent(20);

    ImGui::Spacing();
    ImGui::Spacing();

    // 变换工具
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "变换工具");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::Indent(20);
    ImGui::BulletText("Ctrl + T");
    ImGui::Indent(20);
    ImGui::TextDisabled("进入/退出变换模式");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("拖拽图片");
    ImGui::Indent(20);
    ImGui::TextDisabled("移动图片位置");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("拖拽控制点");
    ImGui::Indent(20);
    ImGui::TextDisabled("等比例缩放图片");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("Ctrl + Shift + 鼠标滚轮");
    ImGui::Indent(20);
    ImGui::TextDisabled("以鼠标为中心缩放图片本身");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("Enter");
    ImGui::Indent(20);
    ImGui::TextDisabled("确认变换");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("ESC");
    ImGui::Indent(20);
    ImGui::TextDisabled("取消变换");
    ImGui::Unindent(20);
    ImGui::Spacing();
    ImGui::Unindent(20);

    ImGui::Spacing();
    ImGui::Spacing();

    // 矩形选框工具
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "矩形选框工具");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::Indent(20);
    ImGui::BulletText("M");
    ImGui::Indent(20);
    ImGui::TextDisabled("进入/退出选框模式");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("拖拽鼠标");
    ImGui::Indent(20);
    ImGui::TextDisabled("创建矩形选区");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("Shift");
    ImGui::Indent(20);
    ImGui::TextDisabled("等比例正方形选区");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("Alt");
    ImGui::Indent(20);
    ImGui::TextDisabled("从中心向外创建选区");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("Shift + Alt");
    ImGui::Indent(20);
    ImGui::TextDisabled("中心对称正方形选区");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("点击选区内部");
    ImGui::Indent(20);
    ImGui::TextDisabled("移动选区位置");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("ESC");
    ImGui::Indent(20);
    ImGui::TextDisabled("取消选区");
    ImGui::Unindent(20);
    ImGui::Spacing();
    ImGui::Unindent(20);

    ImGui::Spacing();
    ImGui::Spacing();

    // 对齐辅助
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "对齐辅助");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::Indent(20);
    ImGui::BulletText("智能对齐");
    ImGui::Indent(20);
    ImGui::TextDisabled("拖拽图片时，接近画布中心会自动显示辅助线");
    ImGui::Unindent(20);
    ImGui::Spacing();

    ImGui::BulletText("磁性吸附");
    ImGui::Indent(20);
    ImGui::TextDisabled("图片中心接近画布中心时会自动吸附对齐");
    ImGui::Unindent(20);
    ImGui::Spacing();
    ImGui::Unindent(20);

    ImGui::Spacing();
    ImGui::Spacing();

    // 提示
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.3f, 1.0f), "提示");
    ImGui::Spacing();
    ImGui::Indent(20);
    ImGui::TextWrapped("• 所有画布导航功能（缩放、平移）在变换模式下也可以使用");
    ImGui::Spacing();
    ImGui::TextWrapped("• 按住空格键时，变换工具会暂时切换为抓手工具");
    ImGui::Spacing();
    ImGui::TextWrapped("• 对齐辅助线的吸附阈值为 8 像素");
    ImGui::Unindent(20);

    ImGui::PopStyleVar();
}

void SettingsPanel::RenderAboutPage() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));

    ImGui::SetWindowFontScale(1.3f);
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "关于程序");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "Batch Processor Pro");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();
    ImGui::TextDisabled("版本 1.0.0");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.1f);
    ImGui::TextWrapped("专业级图片批量处理工具，提供类似 Photoshop 的画布系统和直观的操作体验。");
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "主要功能");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::Indent(20);
    ImGui::BulletText("画布系统（类似 Photoshop）");
    ImGui::Spacing();
    ImGui::BulletText("可视化变换与批量应用");
    ImGui::Spacing();
    ImGui::BulletText("智能对齐辅助线");
    ImGui::Spacing();
    ImGui::BulletText("矩形选框工具（Photoshop 级）");
    ImGui::Spacing();
    ImGui::BulletText("多线程并行处理");
    ImGui::Spacing();
    ImGui::BulletText("支持 JPG/PNG/WEBP 格式");
    ImGui::Unindent(20);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "技术栈");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Spacing();

    ImGui::Indent(20);
    ImGui::BulletText("C++17");
    ImGui::Spacing();
    ImGui::BulletText("Dear ImGui (UI 框架)");
    ImGui::Spacing();
    ImGui::BulletText("OpenGL (图形渲染)");
    ImGui::Spacing();
    ImGui::BulletText("GLFW (窗口管理)");
    ImGui::Spacing();
    ImGui::BulletText("stb_image (图像加载)");
    ImGui::Unindent(20);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextDisabled("© 2026(祝我发大财！！！) Batch Processor Pro. All rights reserved.");

    ImGui::PopStyleVar();
}





