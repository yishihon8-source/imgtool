#pragma once

#include <memory>

struct GLFWwindow;
class MainUI;

/**
 * @brief 应用程序主类
 * 
 * 职责：
 * - 初始化 GLFW + OpenGL + ImGui
 * - 管理主窗口生命周期
 * - 主循环调度
 * - 资源清理
 */
class App {
public:
    App();
    ~App();

    // 禁止拷贝
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /**
     * @brief 初始化应用程序
     * @return 成功返回 true
     */
    bool Initialize();

    /**
     * @brief 运行主循环
     */
    void Run();

    /**
     * @brief 关闭应用程序
     */
    void Shutdown();

private:
    /**
     * @brief 初始化 GLFW
     */
    bool InitializeGLFW();

    /**
     * @brief 初始化 ImGui
     */
    bool InitializeImGui();

    /**
     * @brief 设置现代化样式
     */
    void SetupModernStyle();

    /**
     * @brief 处理一帧
     */
    void ProcessFrame();

    /**
     * @brief 渲染
     */
    void Render();

    /**
     * @brief 文件拖拽回调
     */
    static void DropCallback(GLFWwindow* window, int count, const char** paths);

private:
    GLFWwindow* m_Window = nullptr;
    std::unique_ptr<MainUI> m_MainUI;
    
    // 窗口配置
    const int m_WindowWidth = 1600;
    const int m_WindowHeight = 900;
    const char* m_WindowTitle = "一十不意识 - 高性能图片批量处理工具";
};
