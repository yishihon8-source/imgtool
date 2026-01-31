#pragma once

#include "core/Types.h"
#include "task/BatchProcessor.h"
#include <memory>
#include <vector>

class ImageListPanel;
class PreviewPanel;
class ControlPanel;
class SettingsPanel;

/**
 * @brief 主 UI 类
 * 
 * 职责：
 * - 管理 ImGui Docking 布局
 * - 协调各个 UI 面板
 * - 管理全局状态
 */
class MainUI {
public:
    MainUI();
    ~MainUI();

    /**
     * @brief 渲染 UI
     */
    void Render();

    /**
     * @brief 处理拖拽文件
     */
    void OnFilesDropped(const std::vector<std::string>& filePaths);

private:
    /**
     * @brief 设置专业风格
     */
    void SetupProfessionalStyle();

private:
    /**
     * @brief 设置 Docking 布局
     */
    void SetupDockSpace();

    /**
     * @brief 渲染顶部栏
     */
    void RenderTopBar();
    
    /**
     * @brief 渲染左侧工具栏
     */
    void RenderToolbar();

    /**
     * @brief 渲染底部状态栏
     */
    void RenderBottomStatusBar();

    /**
     * @brief 渲染关于对话框
     */
    void RenderAboutDialog();

    /**
     * @brief 开始批量处理
     */
    void StartBatchProcess();

    /**
     * @brief 添加图片文件
     */
    void AddImages();

    /**
     * @brief 添加文件夹中的所有图片
     */
    void AddImagesFromFolder();

    /**
     * @brief 清空所有图片素材
     */
    void ClearAllImages();

    /**
     * @brief 打开文件资源管理器
     */
    void OpenFileExplorer();

    /**
     * @brief 渲染控制面板（内联实现）
     */
    void RenderControlPanel(ProcessConfig& config);
    void RenderTransformSection(Canvas& canvas);
    void RenderExportSection(OutputFormat& format, int& jpgQuality);
    
    /**
     * @brief 渲染提示对话框
     */
    void RenderNotificationDialog();
    
    /**
     * @brief 显示错误提示
     */
    void ShowError(const std::string& message);
    
    /**
     * @brief 显示成功提示
     */
    void ShowSuccess(const std::string& message);
    
    /**
     * @brief 播放系统声音（成功/错误）
     */
    void PlaySystemSound(bool isSuccess);

    /**
     * @brief 显示批量处理完成对话框
     */
    void ShowBatchProcessComplete(const std::string& message);

    /**
     * @brief 渲染批量处理完成对话框
     */
    void RenderBatchProcessCompleteDialog();

private:
    // UI 面板
    std::unique_ptr<ImageListPanel> m_ImageListPanel;
    std::unique_ptr<PreviewPanel> m_PreviewPanel;
    std::unique_ptr<ControlPanel> m_ControlPanel;
    std::unique_ptr<SettingsPanel> m_SettingsPanel;

    // 批量处理器
    std::unique_ptr<BatchProcessor> m_BatchProcessor;

    // 全局状态
    std::vector<ImageInfo> m_ImageList;
    int m_CurrentImageIndex = -1;
    ProcessConfig m_ProcessConfig;
    
    // 画布状态
    bool m_CanvasApplied = false;  // 画布是否已应用
    Canvas m_TempCanvas;  // 临时画布设置（未应用前）

    // 输出目录
    std::string m_OutputDirectory = "";  // 当前选择的输出目录

    // 提示对话框状态
    enum class NotificationType {
        None,
        Error,
        Success
    };
    NotificationType m_NotificationType = NotificationType::None;
    std::string m_NotificationMessage = "";
    float m_NotificationTimer = 0.0f;  // 用于自动关闭（3秒后自动关闭）
    bool m_ShowNotification = false;

    // 批量处理完成对话框状态
    bool m_ShowBatchProcessComplete = false;
    std::string m_BatchProcessMessage = "";
    bool m_BatchCompletePopupOpened = false;

    // UI 状态
    bool m_ShowAbout = false;
    bool m_ShowSettings = false;  // 是否显示设置面板
    
    // 工具栏状态
    enum class ToolMode {
        None,
        Transform,  // 变换工具
        Selection   // 矩形选框工具
    };
    ToolMode m_CurrentTool = ToolMode::None;
};
