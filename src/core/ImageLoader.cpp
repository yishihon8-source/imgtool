#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include "../utils/Logger.h"

namespace fs = std::filesystem;

bool ImageLoader::Load(const std::string& filePath, ImageData& outData) {
    try {
        Logger::Debug("Load() called for: " + filePath);
        
        // 验证文件存在
        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
            Logger::Error("File does not exist or is not regular: " + filePath);
            return false;
        }
        Logger::Debug("File exists and is regular: " + filePath);

        // 检查文件大小
        try {
            if (fs::file_size(filePath) == 0) {
                Logger::Error("File is empty: " + filePath);
                return false;
            }
            Logger::Debug("File size check passed");
        } catch (const std::exception& e) {
            Logger::Error("Error checking file size: " + std::string(e.what()));
            return false;
        }

        Logger::Debug("Calling stbi_load()...");
        int width, height, channels;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

        if (!data) {
            Logger::Error("stbi_load() failed for: " + filePath);
            Logger::Error("STB Error: " + std::string(stbi_failure_reason()));
            return false;
        }
        Logger::Debug("stbi_load() succeeded: " + std::to_string(width) + "x" + std::to_string(height) + 
                      " channels=" + std::to_string(channels));

        // 验证图片数据
        if (width <= 0 || height <= 0 || channels <= 0) {
            Logger::Error("Invalid image dimensions: " + std::to_string(width) + "x" + std::to_string(height) + 
                         " channels=" + std::to_string(channels));
            stbi_image_free(data);
            return false;
        }

        outData.width = width;
        outData.height = height;
        outData.channels = channels;
        
        Logger::Debug("Allocating pixel memory: " + std::to_string(width * height * channels) + " bytes");
        try {
            outData.pixels.assign(data, data + (width * height * channels));
            Logger::Debug("Pixel memory allocated successfully");
        } catch (const std::exception& e) {
            Logger::Error("Failed to allocate memory for image: " + std::string(e.what()));
            stbi_image_free(data);
            return false;
        }

        Logger::Debug("Freeing STB image data...");
        stbi_image_free(data);
        
        Logger::Info("Load() succeeded for: " + filePath);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Exception in ImageLoader::Load: " + std::string(e.what()));
        return false;
    } catch (...) {
        Logger::Error("Unknown exception in ImageLoader::Load");
        return false;
    }
}

bool ImageLoader::GetInfo(const std::string& filePath, ImageInfo& outInfo) {
    try {
        Logger::Debug("GetInfo() called for: " + filePath);
        
        // 验证文件存在
        if (!fs::exists(filePath)) {
            Logger::Error("File does not exist: " + filePath);
            return false;
        }
        Logger::Debug("File exists: " + filePath);

        if (!fs::is_regular_file(filePath)) {
            Logger::Error("Not a regular file: " + filePath);
            return false;
        }
        Logger::Debug("Is regular file: " + filePath);

        int width, height, channels;
        Logger::Debug("Calling stbi_info()...");
        if (!stbi_info(filePath.c_str(), &width, &height, &channels)) {
            Logger::Error("stbi_info() failed for: " + filePath);
            return false;
        }
        Logger::Debug("stbi_info() succeeded: " + std::to_string(width) + "x" + std::to_string(height) + 
                      " channels=" + std::to_string(channels));

        // 验证图片尺寸合理性（防止损坏文件导致异常）
        if (width <= 0 || height <= 0 || channels <= 0 || width > 65536 || height > 65536) {
            Logger::Warning("Invalid image dimensions: " + std::to_string(width) + "x" + std::to_string(height) + 
                           " channels=" + std::to_string(channels));
            return false;
        }

        Logger::Debug("Building ImageInfo structure...");
        outInfo.filePath = filePath;
        outInfo.fileName = fs::path(filePath).filename().string();
        outInfo.width = width;
        outInfo.height = height;
        outInfo.channels = channels;
        Logger::Debug("ImageInfo filePath: " + outInfo.filePath);
        Logger::Debug("ImageInfo fileName: " + outInfo.fileName);

        // 获取文件大小
        try {
            outInfo.fileSize = fs::file_size(filePath);
            Logger::Debug("File size: " + std::to_string(outInfo.fileSize) + " bytes");
        } catch (const std::exception& e) {
            Logger::Warning("Error getting file size: " + std::string(e.what()));
            outInfo.fileSize = 0;
        }

        Logger::Info("GetInfo() succeeded for: " + filePath);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Exception in GetInfo(): " + std::string(e.what()));
        return false;
    } catch (...) {
        Logger::Error("Unknown exception in GetInfo()");
        return false;
    }
}

bool ImageLoader::SavePNG(const std::string& filePath, const ImageData& data) {
    if (!data.IsValid()) {
        std::cerr << "Invalid image data" << std::endl;
        return false;
    }

    int result = stbi_write_png(
        filePath.c_str(),
        data.width,
        data.height,
        data.channels,
        data.pixels.data(),
        data.width * data.channels
    );

    if (!result) {
        std::cerr << "Failed to save PNG: " << filePath << std::endl;
        return false;
    }

    return true;
}

bool ImageLoader::SaveJPG(const std::string& filePath, const ImageData& data, int quality) {
    if (!data.IsValid()) {
        std::cerr << "Invalid image data" << std::endl;
        return false;
    }

    // JPG 不支持 alpha 通道，需要转换为 RGB
    const uint8_t* pixelData = data.pixels.data();
    std::vector<uint8_t> rgbData;

    if (data.channels == 4) {
        // RGBA -> RGB
        rgbData.reserve(data.width * data.height * 3);
        for (size_t i = 0; i < data.pixels.size(); i += 4) {
            rgbData.push_back(data.pixels[i]);     // R
            rgbData.push_back(data.pixels[i + 1]); // G
            rgbData.push_back(data.pixels[i + 2]); // B
        }
        pixelData = rgbData.data();
    }

    int channels = (data.channels == 4) ? 3 : data.channels;
    int result = stbi_write_jpg(
        filePath.c_str(),
        data.width,
        data.height,
        channels,
        pixelData,
        quality
    );

    if (!result) {
        std::cerr << "Failed to save JPG: " << filePath << std::endl;
        return false;
    }

    return true;
}

bool ImageLoader::GetFolderImages(const std::string& folderPath, std::vector<ImageInfo>& outInfos) {
    try {
        // 基础检查
        if (folderPath.empty()) {
            std::cerr << "Empty folder path" << std::endl;
            return false;
        }

        // 检查目录是否存在
        std::error_code ec;
        if (!fs::exists(folderPath, ec) || ec) {
            std::cerr << "Folder does not exist: " << folderPath << " - " << ec.message() << std::endl;
            return false;
        }

        // 检查是否真的是目录
        if (!fs::is_directory(folderPath, ec) || ec) {
            std::cerr << "Path is not a directory: " << folderPath << " - " << ec.message() << std::endl;
            return false;
        }

        // 用 error_code 而不是异常来遍历
        int fileCount = 0;
        for (const auto& entry : fs::directory_iterator(folderPath, ec)) {
            if (ec) {
                std::cerr << "Error iterating directory: " << ec.message() << std::endl;
                break;
            }

            try {
                // 跳过目录
                if (entry.is_directory(ec)) {
                    continue;
                }

                if (ec) {
                    std::cerr << "Error checking if entry is directory" << std::endl;
                    continue;
                }

                std::string path = entry.path().string();
                
                // 检查文件大小
                std::uintmax_t fileSize = 0;
                try {
                    fileSize = fs::file_size(path, ec);
                    if (ec || fileSize == 0 || fileSize > 2147483648ULL) {
                        continue;
                    }
                } catch (...) {
                    continue;
                }

                // 检查支持的格式
                if (IsSupportedFormat(path)) {
                    ImageInfo info;
                    try {
                        if (GetInfo(path, info)) {
                            outInfos.push_back(info);
                            fileCount++;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error loading image info: " << path << " - " << e.what() << std::endl;
                        continue;
                    } catch (...) {
                        std::cerr << "Unknown error loading image info: " << path << std::endl;
                        continue;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing file: " << e.what() << std::endl;
                continue;
            } catch (...) {
                std::cerr << "Unknown error processing file" << std::endl;
                continue;
            }
        }

        if (ec) {
            std::cerr << "Final error in directory iteration: " << ec.message() << std::endl;
            // 如果至少读到了一些文件，还是返回成功
            if (fileCount > 0) {
                return true;
            }
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in GetFolderImages: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception in GetFolderImages" << std::endl;
        return false;
    }
}

bool ImageLoader::IsSupportedFormat(const std::string& filePath) {
    std::string ext = GetFileExtension(filePath);
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || 
           ext == ".bmp" || ext == ".tga";
}

std::string ImageLoader::GetFileExtension(const std::string& filePath) {
    std::string ext = fs::path(filePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}
