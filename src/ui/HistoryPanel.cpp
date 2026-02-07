#include "HistoryPanel.h"
#include "PreviewPanel.h"
#include <imgui.h>
#include <algorithm>

HistoryPanel::HistoryPanel() = default;

HistoryPanel::~HistoryPanel() = default;

void HistoryPanel::Render(PreviewPanel* previewPanel, GlobalHistory* globalHistory) {
    if (!previewPanel) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "无可用的预览面板");
        return;
    }

    // 获取历史记录信息
    const ImageHistory& history = previewPanel->GetImageHistory();
    size_t historyCount = history.GetHistoryCount();
    int currentIndex = history.GetCurrentIndex();
    bool canUndo = history.CanUndo();
    bool canRedo = history.CanRedo();

    // 顶部按钮区域
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
    
    // 撤销按钮
    if (!canUndo) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("撤销 (Ctrl+Z)", ImVec2(-1, 0))) {
        previewPanel->Undo();
    }
    if (!canUndo) {
        ImGui::EndDisabled();
    }

    // 重做按钮
    if (!canRedo) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("重做 (Ctrl+Shift+Z)", ImVec2(-1, 0))) {
        previewPanel->Redo();
    }
    if (!canRedo) {
        ImGui::EndDisabled();
    }

    ImGui::PopStyleVar();

    ImGui::Separator();

    // 历史记录状态信息
    size_t totalOperations = historyCount;
    if (globalHistory) {
        totalOperations += globalHistory->GetHistoryCount();
    }
    
    ImGui::Text("历史记录数: %zu", totalOperations);
    if (historyCount > 0) {
        ImGui::Text("图像编辑: %d / %zu", currentIndex + 1, historyCount);
    }

    ImGui::Separator();

    // 历史记录列表
    ImGui::BeginChild("HistoryList", ImVec2(0, 0), true);

    if (totalOperations == 0) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "暂无历史记录");
    } else {
        // 显示全局操作历史（从新到旧）
        if (globalHistory && globalHistory->GetHistoryCount() > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
            ImGui::Text("=== 全局操作 ===");
            ImGui::PopStyleColor();
            
            int globalCurrentIndex = globalHistory->GetCurrentIndex();
            
            for (int i = static_cast<int>(globalHistory->GetHistoryCount()) - 1; i >= 0; i--) {
                std::string description = globalHistory->GetDescription(i);
                
                // 当前状态高亮显示
                bool isCurrent = (i == globalCurrentIndex);
                
                if (isCurrent) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.3f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                }
                
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                
                // 可点击的历史记录项
                std::string buttonLabel = (isCurrent ? "[*] " : "    ") + description;
                if (ImGui::Button(buttonLabel.c_str(), ImVec2(-1, 0))) {
                    globalHistory->RestoreTo(i);
                }
                
                ImGui::PopStyleColor(4);
                
                // 悬停提示
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("点击恢复到此状态");
                    ImGui::EndTooltip();
                }
            }
            
            if (historyCount > 0) {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }
        }
        
        // 显示图像编辑历史记录列表（从新到旧）
        if (historyCount > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 0.9f, 1.0f));
            ImGui::Text("=== 图像编辑 ===");
            ImGui::PopStyleColor();
            
            for (int i = static_cast<int>(historyCount) - 1; i >= 0; i--) {
                std::string description = history.GetDescription(i);
                
                // 当前状态高亮显示
                bool isCurrent = (i == currentIndex);
                
                if (isCurrent) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.3f));
                } else if (i > currentIndex) {
                    // 未来状态（可重做）
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.3f));
                } else {
                    // 过去状态（可撤销到）
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                }
                
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                
                // 可点击的历史记录项
                std::string buttonLabel = (isCurrent ? "[*] " : "    ") + std::to_string(i + 1) + ". " + description;
                if (ImGui::Button(buttonLabel.c_str(), ImVec2(-1, 0))) {
                    // 点击历史记录项，恢复到该状态
                    if (i < currentIndex) {
                        // 需要撤销
                        int steps = currentIndex - i;
                        for (int j = 0; j < steps; j++) {
                            previewPanel->Undo();
                        }
                    } else if (i > currentIndex) {
                        // 需要重做
                        int steps = i - currentIndex;
                        for (int j = 0; j < steps; j++) {
                            previewPanel->Redo();
                        }
                    }
                }
                
                ImGui::PopStyleColor(4);
                
                // 悬停提示
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("点击恢复到此状态");
                    ImGui::EndTooltip();
                }
            }
        }
    }

    ImGui::EndChild();
}

