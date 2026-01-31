#pragma once

#include <imgui.h>
#include <string>

/**
 * @brief 设置面板
 * 
 * 职责：
 * - 显示程序设置
 * - 显示帮助和快捷键说明
 * - 管理设置面板的显示状态
 */
class SettingsPanel {
public:
    SettingsPanel();
    ~SettingsPanel() = default;

    /**
     * @brief 渲染设置面板
     * @param isOpen 是否打开设置面板（引用，可以被关闭）
     */
    void Render(bool& isOpen);

private:
    /**
     * @brief 渲染左侧菜单
     */
    void RenderSideMenu();

    /**
     * @brief 渲染右侧内容区域
     */
    void RenderContent();

    /**
     * @brief 渲染帮助页面
     */
    void RenderHelpPage();

    /**
     * @brief 渲染关于页面
     */
    void RenderAboutPage();

private:
    enum class SettingsPage {
        Help,      // 帮助
        About      // 关于
    };

    SettingsPage m_CurrentPage = SettingsPage::Help;
};





