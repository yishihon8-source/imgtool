#include "FileDialog.h"
#include "Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shobjidl.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#endif

std::string FileDialog::OpenFile(const char* filter) {
    try {
        Logger::Debug("OpenFile() called");
#ifdef _WIN32
        char filename[MAX_PATH] = "";

        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;  // 使用 nullptr 而不是 GetActiveWindow()
        ofn.lpstrFilter = filter ? filter : GetImageFilter();
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = "选择图片文件";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        Logger::Debug("Calling GetOpenFileNameA()...");
        if (GetOpenFileNameA(&ofn)) {
            Logger::Info("File selected: " + std::string(filename));
            return std::string(filename);
        } else {
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                Logger::Warning("GetOpenFileNameA() failed with error: " + std::to_string(err));
            } else {
                Logger::Debug("User cancelled file selection");
            }
        }
#endif
        return "";
    } catch (const std::exception& e) {
        Logger::Error("Exception in OpenFile(): " + std::string(e.what()));
        return "";
    } catch (...) {
        Logger::Error("Unknown exception in OpenFile()");
        return "";
    }
}

std::vector<std::string> FileDialog::OpenFiles(const char* filter) {
    std::vector<std::string> result;

    try {
        Logger::Debug("OpenFiles() called");
#ifdef _WIN32
        char filenames[4096] = "";

        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;  // 使用 nullptr 而不是 GetActiveWindow()
        ofn.lpstrFilter = filter ? filter : GetImageFilter();
        ofn.lpstrFile = filenames;
        ofn.nMaxFile = sizeof(filenames);
        ofn.lpstrTitle = "选择图片文件";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | 
                    OFN_EXPLORER | OFN_NOCHANGEDIR;

        Logger::Debug("Calling GetOpenFileNameA() for multiple files...");
        if (GetOpenFileNameA(&ofn)) {
            std::string directory = filenames;
            char* ptr = filenames + directory.length() + 1;

            if (*ptr == '\0') {
                // 单个文件
                Logger::Debug("Single file selected");
                result.push_back(directory);
            } else {
                // 多个文件
                Logger::Debug("Multiple files selected");
                while (*ptr) {
                    std::string filename = ptr;
                    result.push_back(directory + "\\" + filename);
                    ptr += filename.length() + 1;
                }
            }
            Logger::Info("OpenFiles() returned " + std::to_string(result.size()) + " files");
        } else {
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                Logger::Warning("GetOpenFileNameA() failed with error: " + std::to_string(err));
            } else {
                Logger::Debug("User cancelled file selection");
            }
        }
#endif

        return result;
    } catch (const std::exception& e) {
        Logger::Error("Exception in OpenFiles(): " + std::string(e.what()));
        return result;
    } catch (...) {
        Logger::Error("Unknown exception in OpenFiles()");
        return result;
    }
}

std::string FileDialog::OpenFolder() {
    try {
        Logger::Debug("OpenFolder() called");
#ifdef _WIN32
        std::string result;
        
        // 使用现代的 IFileDialog API（和添加图片一样的对话框）
        Logger::Debug("Using IFileDialog (modern folder picker)");
        
        // 初始化 COM
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        bool comInitialized = SUCCEEDED(hr);
        
        IFileDialog* pfd = nullptr;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        
        if (SUCCEEDED(hr)) {
            // 设置为文件夹选择模式
            DWORD dwOptions;
            hr = pfd->GetOptions(&dwOptions);
            if (SUCCEEDED(hr)) {
                hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
            }
            
            // 显示对话框
            hr = pfd->Show(nullptr);
            
            if (SUCCEEDED(hr)) {
                IShellItem* psi = nullptr;
                hr = pfd->GetResult(&psi);
                
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath = nullptr;
                    hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                    
                    if (SUCCEEDED(hr)) {
                        // 转换 WCHAR 到 char（使用 ANSI 编码避免乱码）
                        int size = WideCharToMultiByte(CP_ACP, 0, pszPath, -1, nullptr, 0, nullptr, nullptr);
                        if (size > 0) {
                            std::vector<char> buffer(size);
                            WideCharToMultiByte(CP_ACP, 0, pszPath, -1, buffer.data(), size, nullptr, nullptr);
                            result = buffer.data();
                            Logger::Info("Folder selected: " + result);
                        }
                        CoTaskMemFree(pszPath);
                    }
                    psi->Release();
                }
            } else {
                Logger::Debug("User cancelled folder selection");
            }
            
            pfd->Release();
        } else {
            Logger::Warning("Failed to create IFileDialog, COM may not be initialized");
        }
        
        // 清理 COM
        if (comInitialized) {
            CoUninitialize();
        }
        
        return result;
#else
        return "";
#endif
    } catch (const std::exception& e) {
        Logger::Error("Exception in OpenFolder(): " + std::string(e.what()));
        return "";
    } catch (...) {
        Logger::Error("Unknown exception in OpenFolder()");
        return "";
    }
}

std::string FileDialog::SaveFile(const char* filter, const char* defaultExt) {
    try {
        Logger::Debug("SaveFile() called");
#ifdef _WIN32
        char filename[MAX_PATH] = "";

        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = filter ? filter : GetImageFilter();
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = defaultExt;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        Logger::Debug("Calling GetSaveFileNameA()...");
        if (GetSaveFileNameA(&ofn)) {
            Logger::Info("Save location selected: " + std::string(filename));
            return std::string(filename);
        } else {
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                Logger::Warning("GetSaveFileNameA() failed with error: " + std::to_string(err));
            } else {
                Logger::Debug("User cancelled save dialog");
            }
        }
#endif
        return "";
    } catch (const std::exception& e) {
        Logger::Error("Exception in SaveFile(): " + std::string(e.what()));
        return "";
    } catch (...) {
        Logger::Error("Unknown exception in SaveFile()");
        return "";
    }
}

const char* FileDialog::GetImageFilter() {
    return "Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.tga\0"
           "All Files\0*.*\0";
}

void FileDialog::OpenInExplorer(const std::string& folderPath) {
    try {
        Logger::Debug("OpenInExplorer() called");
#ifdef _WIN32
        if (folderPath.empty()) {
            Logger::Debug("Opening explorer to My Computer");
            // 打开"我的电脑"
            ShellExecuteA(nullptr, "open", "explorer.exe", "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", nullptr, SW_SHOWNORMAL);
        } else {
            Logger::Debug("Opening explorer to: " + folderPath);
            // 打开指定文件夹
            ShellExecuteA(nullptr, "open", "explorer.exe", folderPath.c_str(), nullptr, SW_SHOWNORMAL);
        }
#endif
    } catch (const std::exception& e) {
        Logger::Error("Exception in OpenInExplorer(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in OpenInExplorer()");
    }
}
