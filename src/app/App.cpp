#include "App.h"
#include "ui/MainUI.h"
#include "utils/Logger.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

// GLFW 错误回调
static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    Logger::Error(std::string("GLFW Error ") + std::to_string(error) + ": " + description);
}

App::App() = default;
App::~App() = default;

bool App::Initialize() {
    try {
        Logger::Info("Initializing GLFW...");
        if (!InitializeGLFW()) {
            Logger::Error("GLFW initialization failed");
            return false;
        }
        Logger::Info("GLFW initialized successfully");

        Logger::Info("Initializing ImGui...");
        if (!InitializeImGui()) {
            Logger::Error("ImGui initialization failed");
            return false;
        }
        Logger::Info("ImGui initialized successfully");

        // 创建主 UI
        Logger::Info("Creating MainUI instance...");
        m_MainUI = std::make_unique<MainUI>();
        Logger::Info("MainUI created successfully");

        return true;
    } catch (const std::exception& e) {
        Logger::Error(std::string("Exception during Initialize: ") + e.what());
        return false;
    } catch (...) {
        Logger::Error("Unknown exception during Initialize");
        return false;
    }
}

bool App::InitializeGLFW() {
    try {
        Logger::Debug("Setting GLFW error callback...");
        glfwSetErrorCallback(GLFWErrorCallback);

        Logger::Debug("Initializing GLFW...");
        if (!glfwInit()) {
            Logger::Error("glfwInit() failed");
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        Logger::Debug("glfwInit() succeeded");

        // OpenGL 3.3 Core Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // 创建窗口
        Logger::Debug("Creating GLFW window...");
        m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle, nullptr, nullptr);
        if (!m_Window) {
            Logger::Error("glfwCreateWindow() failed");
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        Logger::Debug("GLFW window created successfully");

        glfwMakeContextCurrent(m_Window);
        glfwSwapInterval(1); // 启用 VSync

        // 设置拖拽回调
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetDropCallback(m_Window, DropCallback);

        Logger::Debug("GLFW initialization completed");
        return true;
    } catch (const std::exception& e) {
        Logger::Error(std::string("Exception in InitializeGLFW: ") + e.what());
        return false;
    } catch (...) {
        Logger::Error("Unknown exception in InitializeGLFW");
        return false;
    }
}

bool App::InitializeImGui() {
    try {
        Logger::Debug("Creating ImGui context...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        Logger::Debug("ImGui context created successfully");
        
        // 禁用配置文件保存
        io.IniFilename = nullptr;  // 不保存 imgui.ini
        
        // 启用 Docking 和 Viewport
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // 加载中文字体 - 大幅增大字体
        Logger::Debug("Configuring fonts...");
        ImFontConfig font_cfg;
        font_cfg.OversampleH = 2;
        font_cfg.OversampleV = 2;
        font_cfg.PixelSnapH = true;
        
        // 尝试加载系统中文字体，如果失败使用默认字体
        ImFont* font = nullptr;
        std::vector<std::string> font_paths = {
            "C:\\Windows\\Fonts\\msyh.ttc",      // 微软雅黑
            "C:\\Windows\\Fonts\\simhei.ttf",    // 黑体
            "C:\\Windows\\Fonts\\arial.ttf"      // Arial 作为备选
        };
        
        bool font_loaded = false;
        for (const auto& path : font_paths) {
            Logger::Debug("Trying to load font: " + path);
            try {
                font = io.Fonts->AddFontFromFileTTF(path.c_str(), 22.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
                if (font != nullptr) {
                    Logger::Info("Successfully loaded font from: " + path);
                    font_loaded = true;
                    break;
                }
            } catch (const std::exception& e) {
                Logger::Warning("Font loading failed: " + std::string(e.what()));
                continue;
            } catch (...) {
                Logger::Warning("Unknown error loading font");
                continue;
            }
        }
        
        if (!font_loaded) {
            Logger::Warning("Could not load any font file, using default font");
        }

        Logger::Debug("Setting up style...");
        SetupModernStyle();

        // 当启用 Viewport 时，调整样式
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        
        // 初始化平台/渲染器后端
        Logger::Debug("Initializing ImGui backends...");
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        Logger::Debug("ImGui backends initialized successfully");

        Logger::Debug("ImGui initialization completed");
        return true;
    } catch (const std::exception& e) {
        Logger::Error(std::string("Exception in InitializeImGui: ") + e.what());
        return false;
    } catch (...) {
        Logger::Error("Unknown exception in InitializeImGui");
        return false;
    }
}

void App::SetupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // 圆角和边框 - 更大的圆角
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    // 边框
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    // 大幅增加所有间距和尺寸
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(16, 10);
    style.ItemSpacing = ImVec2(16, 12);
    style.ItemInnerSpacing = ImVec2(10, 8);
    style.IndentSpacing = 30.0f;
    style.ScrollbarSize = 18.0f;
    style.GrabMinSize = 14.0f;

    // 现代化浅色主题 - 白色为主
    colors[ImGuiCol_Text]                   = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.15f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.96f, 0.96f, 0.96f, 0.75f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.16f, 0.49f, 0.88f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
    colors[ImGuiCol_Separator]              = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.76f, 0.80f, 0.84f, 0.93f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.60f, 0.73f, 0.88f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.92f, 0.93f, 0.94f, 0.99f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.74f, 0.82f, 0.91f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.22f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void App::Run() {
    try {
        while (!glfwWindowShouldClose(m_Window)) {
            try {
                ProcessFrame();
            } catch (const std::exception& e) {
                std::cerr << "Exception in ProcessFrame: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown exception in ProcessFrame loop" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in Run: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in Run" << std::endl;
    }
}

void App::ProcessFrame() {
    try {
        // 轮询事件
        glfwPollEvents();

        // 开始新的 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 渲染主 UI
        try {
            if (m_MainUI) {
                m_MainUI->Render();
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in MainUI::Render: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in MainUI::Render" << std::endl;
        }

        // 渲染
        Render();
    } catch (const std::exception& e) {
        std::cerr << "Exception in ProcessFrame: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in ProcessFrame" << std::endl;
    }
}

void App::Render() {
    try {
        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // 清屏
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 渲染 ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 更新和渲染额外的平台窗口
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(m_Window);
    } catch (const std::exception& e) {
        std::cerr << "Exception in Render: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in Render" << std::endl;
    }
}

void App::Shutdown() {
    // 清理 ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 清理 GLFW
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
}

void App::DropCallback(GLFWwindow* window, int count, const char** paths) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app && app->m_MainUI) {
        // 将拖拽的文件路径传递给 MainUI
        std::vector<std::string> filePaths;
        for (int i = 0; i < count; i++) {
            filePaths.push_back(paths[i]);
        }
        app->m_MainUI->OnFilesDropped(filePaths);
    }
}
