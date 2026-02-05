cmake_minimum_required(VERSION 3.15)
project(ImageBatchTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== 商业级发布配置：使用动态链接（降低误报率）=====
if(WIN32 AND MSVC)
    # 动态链接 MSVC 运行时库（推荐用于发布）
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # 动态链接标准库
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    
    # 链接器选项
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
endif()

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 第三方库路径
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# ImGui
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# GLFW - 静态链接（第三方库可以静态链接）
set(GLFW_DIR ${THIRD_PARTY_DIR}/glfw)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_SHARED_LIBS OFF)  # GLFW 静态链接
option(BUILD_SHARED_LIBS OFF)       # 第三方库静态链接
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON)  # GLFW 使用动态运行时
add_subdirectory(${GLFW_DIR})

# STB (header-only)
set(STB_DIR ${THIRD_PARTY_DIR}/stb)

# OpenGL
find_package(OpenGL REQUIRED)

# 源文件
set(PROJECT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/app/App.cpp
    ${CMAKE_SOURCE_DIR}/src/app/App.h
    ${CMAKE_SOURCE_DIR}/src/core/ImageLoader.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ImageLoader.h
    ${CMAKE_SOURCE_DIR}/src/core/ImageProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ImageProcessor.h
    ${CMAKE_SOURCE_DIR}/src/core/TransformManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/TransformManager.h
    ${CMAKE_SOURCE_DIR}/src/core/GuideLineManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/GuideLineManager.h
    ${CMAKE_SOURCE_DIR}/src/core/SelectionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SelectionSystem.h
    ${CMAKE_SOURCE_DIR}/src/core/SelectionHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SelectionHistory.h
    ${CMAKE_SOURCE_DIR}/src/core/SelectionMath.cpp
    ${CMAKE_SOURCE_DIR}/src/core/SelectionMath.h
    ${CMAKE_SOURCE_DIR}/src/core/OutOfBoundsRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/OutOfBoundsRenderer.h
    ${CMAKE_SOURCE_DIR}/src/core/ImageHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/ImageHistory.h
    ${CMAKE_SOURCE_DIR}/src/core/Types.h
    ${CMAKE_SOURCE_DIR}/src/task/BatchProcessor.cpp
    ${CMAKE_SOURCE_DIR}/src/task/BatchProcessor.h
    ${CMAKE_SOURCE_DIR}/src/task/ThreadPool.cpp
    ${CMAKE_SOURCE_DIR}/src/task/ThreadPool.h
    ${CMAKE_SOURCE_DIR}/src/ui/ControlPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ControlPanel.h
    ${CMAKE_SOURCE_DIR}/src/ui/ImageListPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ImageListPanel.h
    ${CMAKE_SOURCE_DIR}/src/ui/MainUI.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/MainUI.h
    ${CMAKE_SOURCE_DIR}/src/ui/PreviewPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/PreviewPanel.h
    ${CMAKE_SOURCE_DIR}/src/ui/SettingsPanel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/SettingsPanel.h
    ${CMAKE_SOURCE_DIR}/src/utils/FileDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/FileDialog.h
    ${CMAKE_SOURCE_DIR}/src/utils/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/Logger.h
)

# 可执行文件
add_executable(${PROJECT_NAME} 
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLFW_DIR}/include
    ${STB_DIR}
    ${OPENGL_INCLUDE_DIR}
)

# 启用 ImGui Docking 功能
target_compile_definitions(${PROJECT_NAME} PRIVATE
    IMGUI_ENABLE_DOCKING
    IMGUI_ENABLE_VIEWPORTS
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
)

# Windows 多媒体库（用于系统声音）
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
endif()

# Windows 特定设置
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
    # 隐藏控制台窗口（所有模式）
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # 添加应用程序图标和版本信息（降低误报率）
    if(EXISTS ${CMAKE_SOURCE_DIR}/resources/app.rc)
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/resources/app.rc)
    endif()
endif()

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MP /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
