#include "ControlPanel.h"
#include <imgui.h>

void ControlPanel::Render(ProcessConfig& config) {
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Width: %d", config.canvas.width);
    ImGui::Text("Height: %d", config.canvas.height);
    ImGui::End();
}

void ControlPanel::RenderTransformSection(Canvas& canvas) {
    (void)canvas;  // 避免未使用参数警告
}

void ControlPanel::RenderAdjustmentsSection(ScaleMode& mode) {
    (void)mode;  // 避免未使用参数警告
}

void ControlPanel::RenderWatermarkSection() {
}

void ControlPanel::RenderExportSection(OutputFormat& format, int& jpgQuality) {
    (void)format;  // 避免未使用参数警告
    (void)jpgQuality;  // 避免未使用参数警告
}

void ControlPanel::RenderCanvasSection(Canvas& canvas) {
    (void)canvas;  // 避免未使用参数警告
}

void ControlPanel::RenderImageSection(CropParams& crop) {
    (void)crop;  // 避免未使用参数警告
}

void ControlPanel::RenderLayoutSection(ScaleMode& mode, Alignment& alignment) {
    (void)mode;  // 避免未使用参数警告
    (void)alignment;  // 避免未使用参数警告
}
