#include "ImageListPanel.h"
#include "core/ImageLoader.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>

// OpenGL
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

ImageListPanel::~ImageListPanel() {
    ClearThumbnails();
}

void ImageListPanel::Render(std::vector<ImageInfo>& imageList, int& currentIndex, 
                            std::function<void()> onAddFiles,
                            std::function<void()> onOpenFolder,
                            std::function<void()> onClearAll) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));  // 增大内边距
    ImGui::Begin("素材列表", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar();

    // 标题栏：素材列表 + 清空按钮 + 添加按钮
    ImGui::SetWindowFontScale(1.3f);  // 标题字体放大
    ImGui::Text("素材列表");
    ImGui::SetWindowFontScale(1.0f);
    
    // 清空按钮（只在有素材时显示）
    if (!imageList.empty()) {
        ImGui::SameLine(ImGui::GetWindowWidth() - 130);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        
        ImGui::SetWindowFontScale(1.1f);
        if (ImGui::Button("清空", ImVec2(48, 36))) {
            ImGui::OpenPopup("ClearAllConfirm");
        }
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::PopStyleColor(3);
    }
    
    // 添加按钮
    ImGui::SameLine(ImGui::GetWindowWidth() - 70);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    ImGui::SetWindowFontScale(1.5f);  // + 号放大
    if (ImGui::Button("+", ImVec2(48, 36))) {  // 按钮放大
        ImGui::OpenPopup("AddImageMenu");
    }
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::PopStyleColor(3);
    
    // 添加图片菜单
    if (ImGui::BeginPopup("AddImageMenu")) {
        ImGui::SetWindowFontScale(1.1f);
        if (ImGui::MenuItem("导入图片...")) {
            if (onAddFiles) {
                onAddFiles();
            }
        }
        if (ImGui::MenuItem("导入文件夹...")) {
            if (onOpenFolder) {
                onOpenFolder();
            }
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::EndPopup();
    }
    
    ImGui::Spacing();
    ImGui::Spacing();

    // 搜索框
    static char searchBuffer[256] = "";
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));  // 增大搜索框内边距
    ImGui::SetNextItemWidth(-1);
    ImGui::SetWindowFontScale(1.1f);  // 搜索框字体放大
    ImGui::InputTextWithHint("##Search", "搜索文件名", searchBuffer, IM_ARRAYSIZE(searchBuffer));
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();

    // 图片列表
    ImGui::BeginChild("##ImageList", ImVec2(0, 0), false);

    if (imageList.empty()) {
        // 空状态
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetCursorPosY(windowSize.y * 0.4f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        
        ImGui::SetWindowFontScale(1.3f);  // 空状态文字放大
        float textWidth = ImGui::CalcTextSize("暂无素材").x;
        ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
        ImGui::TextUnformatted("暂无素材");
        
        ImGui::Spacing();
        
        ImGui::SetWindowFontScale(1.1f);
        textWidth = ImGui::CalcTextSize("点击 + 添加").x;
        ImGui::SetCursorPosX((windowSize.x - textWidth) * 0.5f);
        ImGui::TextUnformatted("点击 + 添加");
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::PopStyleColor();
    } else {
        for (int i = 0; i < static_cast<int>(imageList.size()); ++i) {
            ImGui::PushID(i);
            
            bool isSelected = (i == currentIndex);
            
            // 渲染图片项
            bool clicked = RenderImageItem(imageList[i], i, isSelected);
            
            // 保存 Selectable 的状态
            bool itemHovered = ImGui::IsItemHovered();
            (void)itemHovered;  // 避免未使用警告
            
            // 单击选中
            if (clicked) {
                currentIndex = i;
            }
            
            // 双击重命名 - 必须在 IsItemHovered 之后立即检测
            if (itemHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                m_RenamingIndex = i;
                // 提取文件名（不含扩展名）
                std::string fileName = imageList[i].fileName;
                size_t dotPos = fileName.find_last_of('.');
                if (dotPos != std::string::npos) {
                    fileName = fileName.substr(0, dotPos);
                }
                strncpy_s(m_RenameBuffer, fileName.c_str(), sizeof(m_RenameBuffer) - 1);
                m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                // 标记为双击触发，直接打开重命名对话框
                m_DoubleClickTriggered = true;
            }
            
            // 右键菜单 - 只记录状态，在循环外打开
            if (itemHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                m_RenamingIndex = i;
                currentIndex = i;  // 右键时也选中该项
                std::string fileName = imageList[i].fileName;
                size_t dotPos = fileName.find_last_of('.');
                if (dotPos != std::string::npos) {
                    fileName = fileName.substr(0, dotPos);
                }
                strncpy_s(m_RenameBuffer, fileName.c_str(), sizeof(m_RenameBuffer) - 1);
                m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
                m_RightClickTriggered = true;  // 标记右键触发
            }
            
            // 拖拽排序
            if (m_RenamingIndex != i) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("IMAGE_REORDER", &i, sizeof(int));
                    ImGui::Text("移动: %s", imageList[i].fileName.c_str());
                    m_DraggedIndex = i;
                    ImGui::EndDragDropSource();
                }
            }
            
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("IMAGE_REORDER")) {
                    int sourceIndex = *(const int*)payload->Data;
                    int targetIndex = i;
                    
                    // 交换位置
                    if (sourceIndex != targetIndex) {
                        ImageInfo temp = imageList[sourceIndex];
                        imageList.erase(imageList.begin() + sourceIndex);
                        imageList.insert(imageList.begin() + targetIndex, temp);
                        
                        // 更新选中索引
                        if (currentIndex == sourceIndex) {
                            currentIndex = targetIndex;
                        } else if (sourceIndex < currentIndex && targetIndex >= currentIndex) {
                            currentIndex--;
                        } else if (sourceIndex > currentIndex && targetIndex <= currentIndex) {
                            currentIndex++;
                        }
                        
                        m_DraggedIndex = -1;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            
            ImGui::PopID();
            
            // 添加更大的间距，避免选中背景重叠
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
        }
    }

    ImGui::EndChild();
    
    // 右键触发打开菜单（在循环外，ID 栈正确）
    if (m_RightClickTriggered) {
        ImGui::OpenPopup("RenameContextMenu");
        m_RightClickTriggered = false;
    }
    
    // 右键菜单（必须在 EndChild 之后）
    if (ImGui::BeginPopup("RenameContextMenu")) {
        if (ImGui::MenuItem("重命名")) {
            m_OpenRenameDialog = true;  // 标记需要打开重命名对话框
            ImGui::CloseCurrentPopup();  // 关闭右键菜单
        }
        if (ImGui::MenuItem("删除")) {
            m_OpenDeleteDialog = true;  // 标记需要打开删除对话框
            ImGui::CloseCurrentPopup();  // 关闭右键菜单
        }
        ImGui::EndPopup();
    }
    
    // 在菜单外打开模态对话框
    if (m_OpenRenameDialog) {
        ImGui::OpenPopup("RenameDialog");
        m_OpenRenameDialog = false;
    }
    
    if (m_OpenDeleteDialog) {
        ImGui::OpenPopup("DeleteConfirmDialog");
        m_OpenDeleteDialog = false;
    }
    
    // 删除确认对话框 - 居中显示
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("DeleteConfirmDialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("确定要删除这张图片吗？");
        ImGui::Spacing();
        
        if (m_RenamingIndex >= 0 && m_RenamingIndex < static_cast<int>(imageList.size())) {
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "%s", imageList[m_RenamingIndex].fileName.c_str());
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("确定", ImVec2(120, 0))) {
            // 执行删除
            if (m_RenamingIndex >= 0 && m_RenamingIndex < static_cast<int>(imageList.size())) {
                // 清理该图片的缩略图纹理
                std::string filePath = imageList[m_RenamingIndex].filePath;
                auto it = m_ThumbnailCache.find(filePath);
                if (it != m_ThumbnailCache.end()) {
                    if (it->second != 0) {
                        glDeleteTextures(1, &it->second);
                    }
                    m_ThumbnailCache.erase(it);
                }
                
                // 从列表中删除
                imageList.erase(imageList.begin() + m_RenamingIndex);
                
                // 更新当前选中索引
                if (currentIndex == m_RenamingIndex) {
                    // 如果删除的是当前选中项
                    if (currentIndex >= static_cast<int>(imageList.size())) {
                        currentIndex = static_cast<int>(imageList.size()) - 1;
                    }
                } else if (currentIndex > m_RenamingIndex) {
                    // 如果删除的在当前选中项之前，索引需要减1
                    currentIndex--;
                }
                
                m_RenamingIndex = -1;
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("取消", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_RenamingIndex = -1;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // 重命名对话框（模态窗口）- 居中显示
    center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("RenameDialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("重命名文件");
        ImGui::Separator();
        ImGui::Spacing();
        
        // 首次打开时设置焦点
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        
        ImGui::SetNextItemWidth(300);
        bool enterPressed = ImGui::InputText("##RenameInput", m_RenameBuffer, sizeof(m_RenameBuffer), 
                                              ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
        
        ImGui::Spacing();
        
        if (ImGui::Button("确定", ImVec2(140, 0)) || enterPressed) {
            // 确认重命名
            if (m_RenamingIndex >= 0 && m_RenamingIndex < static_cast<int>(imageList.size())) {
                std::string newName = m_RenameBuffer;
                if (!newName.empty()) {
                    // 保留原扩展名
                    std::string oldName = imageList[m_RenamingIndex].fileName;
                    size_t dotPos = oldName.find_last_of('.');
                    if (dotPos != std::string::npos) {
                        newName += oldName.substr(dotPos);
                    }
                    
                    // 只更新显示名称，不修改实际文件路径
                    // 这样可以避免文件找不到的问题
                    imageList[m_RenamingIndex].fileName = newName;
                    
                    // 注意：filePath 保持不变，因为磁盘上的文件没有被重命名
                    // 如果需要真正重命名文件，需要使用 std::filesystem::rename
                }
            }
            m_RenamingIndex = -1;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("取消", ImVec2(140, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_RenamingIndex = -1;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // 双击触发重命名对话框
    if (m_DoubleClickTriggered) {
        ImGui::OpenPopup("RenameDialog");
        m_DoubleClickTriggered = false;
    }
    
    // 清空确认对话框 - 居中显示
    center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("ClearAllConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SetWindowFontScale(1.1f);
        ImGui::Text("确定要清空所有素材吗？");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
        ImGui::SetWindowFontScale(1.05f);
        ImGui::Text("共 %d 张图片将被移除", static_cast<int>(imageList.size()));
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
        
        if (ImGui::Button("确定清空", ImVec2(120, 0))) {
            // 执行清空
            if (onClearAll) {
                onClearAll();
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::PopStyleColor(2);
        
        ImGui::SameLine();
        
        if (ImGui::Button("取消", ImVec2(120, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }

    ImGui::End();
}

bool ImageListPanel::RenderImageItem(const ImageInfo& info, int index, bool isSelected) {
    // 避免未使用参数警告
    (void)index;
    
    // 注意：不要在这里 PushID，因为外部已经 PushID 了

    // 选中状态样式
    if (isSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.45f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.26f, 0.59f, 0.98f, 0.55f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.20f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.16f, 0.16f, 0.16f, 1.0f));
    }

    // 减小高度，避免重叠（从 100 改为 92）
    bool clicked = ImGui::Selectable("##item", isSelected, 0, ImVec2(0, 92));

    ImGui::PopStyleColor(3);

    // 在 Selectable 上使用 DrawList 绘制内容（不会拦截鼠标事件）
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // 缩略图 (76x76)
    unsigned int thumbnailID = GetThumbnailTexture(info.filePath);
    
    if (thumbnailID != 0) {
        // 显示真实的缩略图
        ImVec2 thumbMin(pos.x + 12, pos.y + 12);
        ImVec2 thumbMax(thumbMin.x + 76, thumbMin.y + 76);
        drawList->AddImage((void*)(intptr_t)thumbnailID, thumbMin, thumbMax);
    } else {
        // 显示占位符
        ImVec2 thumbMin(pos.x + 12, pos.y + 12);
        ImVec2 thumbMax(thumbMin.x + 76, thumbMin.y + 76);
        drawList->AddRectFilled(thumbMin, thumbMax, IM_COL32(40, 40, 40, 255));
        drawList->AddRect(thumbMin, thumbMax, IM_COL32(64, 64, 64, 255));
        
        // 缩略图图标文字
        ImVec2 textPos(pos.x + 32, pos.y + 42);
        drawList->AddText(textPos, IM_COL32(100, 100, 100, 255), "IMG");
    }

    // 文件名
    std::string displayName = info.fileName;
    if (displayName.length() > 18) {
        displayName = displayName.substr(0, 15) + "...";
    }
    ImVec2 namePos(pos.x + 100, pos.y + 16);
    drawList->AddText(namePos, IM_COL32(230, 230, 230, 255), displayName.c_str());
    
    // 尺寸信息
    char sizeText[64];
    snprintf(sizeText, sizeof(sizeText), "%dx%d", info.width, info.height);
    ImVec2 sizePos(pos.x + 100, pos.y + 46);
    drawList->AddText(sizePos, IM_COL32(128, 128, 128, 255), sizeText);
    
    return clicked;
}

std::string ImageListPanel::FormatFileSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    return oss.str();
}

unsigned int ImageListPanel::GetThumbnailTexture(const std::string& filePath) {
    // 检查缓存
    auto it = m_ThumbnailCache.find(filePath);
    if (it != m_ThumbnailCache.end()) {
        return it->second;
    }
    
    // 加载图片
    ImageData imageData;
    if (!ImageLoader::Load(filePath, imageData) || !imageData.IsValid()) {
        return 0;
    }
    
    // 创建缩略图纹理（76x76）
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    GLenum format = GL_RGB;
    if (imageData.channels == 4) {
        format = GL_RGBA;
    } else if (imageData.channels == 1) {
        format = GL_RED;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, imageData.width, imageData.height,
                 0, format, GL_UNSIGNED_BYTE, imageData.pixels.data());
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // 缓存纹理
    m_ThumbnailCache[filePath] = textureID;
    
    return textureID;
}

void ImageListPanel::ClearThumbnails() {
    for (auto& pair : m_ThumbnailCache) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }
    m_ThumbnailCache.clear();
}
