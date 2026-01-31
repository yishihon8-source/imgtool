#pragma once

#include "Types.h"
#include <string>
#include <vector>

/**
 * @brief 图像加载器
 * 
 * 职责：
 * - 使用 stb_image 加载图片
 * - 获取图片信息
 * - 保存图片（stb_image_write）
 */
class ImageLoader {
public:
    /**
     * @brief 加载图片
     * @param filePath 文件路径
     * @param outData 输出图像数据
     * @return 成功返回 true
     */
    static bool Load(const std::string& filePath, ImageData& outData);

    /**
     * @brief 获取图片信息（不加载像素数据）
     * @param filePath 文件路径
     * @param outInfo 输出图片信息
     * @return 成功返回 true
     */
    static bool GetInfo(const std::string& filePath, ImageInfo& outInfo);

    /**
     * @brief 保存图片为 PNG
     * @param filePath 文件路径
     * @param data 图像数据
     * @return 成功返回 true
     */
    static bool SavePNG(const std::string& filePath, const ImageData& data);

    /**
     * @brief 保存图片为 JPG
     * @param filePath 文件路径
     * @param data 图像数据
     * @param quality 质量（1-100）
     * @return 成功返回 true
     */
    static bool SaveJPG(const std::string& filePath, const ImageData& data, int quality = 95);

    /**
     * @brief 批量获取文件夹中的图片信息
     * @param folderPath 文件夹路径
     * @param outInfos 输出图片信息列表
     * @return 成功返回 true
     */
    static bool GetFolderImages(const std::string& folderPath, std::vector<ImageInfo>& outInfos);

    /**
     * @brief 检查文件是否为支持的图片格式
     * @param filePath 文件路径
     * @return 支持返回 true
     */
    static bool IsSupportedFormat(const std::string& filePath);

private:
    static std::string GetFileExtension(const std::string& filePath);
};
