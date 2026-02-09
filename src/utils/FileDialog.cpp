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
        wchar_t filename[MAX_PATH] = L"";

        // 转换 filter 到宽字符
        std::wstring wfilter;
        if (filter) {
            int len = MultiByteToWideChar(CP_UTF8, 0, filter, -1, nullptr, 0);
            if (len > 0) {
                wfilter.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, filter, -1, &wfilter[0], len);
            }
        } else {
            const char* defaultFilter = GetImageFilter();
            int len = MultiByteToWideChar(CP_UTF8, 0, defaultFilter, -1, nullptr, 0);
            if (len > 0) {
                wfilter.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, defaultFilter, -1, &wfilter[0], len);
            }
        }

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = wfilter.empty() ? nullptr : wfilter.c_str();
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = L"选择图片文件";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

        Logger::Debug("Calling GetOpenFileNameW()...");
        if (GetOpenFileNameW(&ofn)) {
            // 转换宽字符路径到 UTF-8
            int size = WideCharToMultiByte(CP_UTF8, 0, filename, -1, nullptr, 0, nullptr, nullptr);
            if (size > 0) {
                std::vector<char> buffer(size);
                WideCharToMultiByte(CP_UTF8, 0, filename, -1, buffer.data(), size, nullptr, nullptr);
                std::string result = buffer.data();
                Logger::Info("File selected: " + result);
                return result;
            }
        } else {
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                Logger::Warning("GetOpenFileNameW() failed with error: " + std::to_string(err));
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
        wchar_t filenames[4096] = L"";

        // 转换 filter 到宽字符
        std::wstring wfilter;
        if (filter) {
            int len = MultiByteToWideChar(CP_UTF8, 0, filter, -1, nullptr, 0);
            if (len > 0) {
                wfilter.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, filter, -1, &wfilter[0], len);
            }
        } else {
            const char* defaultFilter = GetImageFilter();
            int len = MultiByteToWideChar(CP_UTF8, 0, defaultFilter, -1, nullptr, 0);
            if (len > 0) {
                wfilter.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, defaultFilter, -1, &wfilter[0], len);
            }
        }

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = wfilter.empty() ? nullptr : wfilter.c_str();
        ofn.lpstrFile = filenames;
        ofn.nMaxFile = sizeof(filenames) / sizeof(wchar_t);
        ofn.lpstrTitle = L"选择图片文件";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | 
                    OFN_EXPLORER | OFN_NOCHANGEDIR;

        Logger::Debug("Calling GetOpenFileNameW() for multiple files...");
        if (GetOpenFileNameW(&ofn)) {
            // 转换宽字符路径到 UTF-8
            auto wcharToUtf8 = [](const wchar_t* wstr) -> std::string {
                int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
                if (size > 0) {
                    std::vector<char> buffer(size);
                    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer.data(), size, nullptr, nullptr);
                    return buffer.data();
                }
                return "";
            };

            std::wstring wdirectory = filenames;
            wchar_t* ptr = filenames + wdirectory.length() + 1;

            if (*ptr == L'\0') {
                // 单个文件
                Logger::Debug("Single file selected");
                result.push_back(wcharToUtf8(filenames));
            } else {
                // 多个文件
                Logger::Debug("Multiple files selected");
                std::string directory = wcharToUtf8(filenames);
                while (*ptr) {
                    std::wstring wfilename = ptr;
                    std::string filename = wcharToUtf8(ptr);
                    result.push_back(directory + "\\" + filename);
                    ptr += wfilename.length() + 1;
                }
            }
            Logger::Info("OpenFiles() returned " + std::to_string(result.size()) + " files");
        } else {
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                Logger::Warning("GetOpenFileNameW() failed with error: " + std::to_string(err));
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
                        // 转换 WCHAR 到 UTF-8 编码（正确支持中文路径）
                        int size = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, nullptr, 0, nullptr, nullptr);
                        if (size > 0) {
                            std::vector<char> buffer(size);
                            WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, buffer.data(), size, nullptr, nullptr);
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
        wchar_t filename[MAX_PATH] = L"";

        // 转换 filter 到宽字符
        std::wstring wfilter;
        if (filter) {
            int len = MultiByteToWideChar(CP_UTF8, 0, filter, -1, nullptr, 0);
            if (len > 0) {
                wfilter.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, filter, -1, &wfilter[0], len);
            }
        } else {
            const char* defaultFilter = GetImageFilter();
            int len = MultiByteToWideChar(CP_UTF8, 0, defaultFilter, -1, nullptr, 0);
            if (len > 0) {
                wfilter.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, defaultFilter, -1, &wfilter[0], len);
            }
        }

        // 转换 defaultExt 到宽字符
        std::wstring wdefaultExt;
        if (defaultExt) {
            int len = MultiByteToWideChar(CP_UTF8, 0, defaultExt, -1, nullptr, 0);
            if (len > 0) {
                wdefaultExt.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, defaultExt, -1, &wdefaultExt[0], len);
            }
        }

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = wfilter.empty() ? nullptr : wfilter.c_str();
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = wdefaultExt.empty() ? nullptr : wdefaultExt.c_str();
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        Logger::Debug("Calling GetSaveFileNameW()...");
        if (GetSaveFileNameW(&ofn)) {
            // 转换宽字符路径到 UTF-8
            int size = WideCharToMultiByte(CP_UTF8, 0, filename, -1, nullptr, 0, nullptr, nullptr);
            if (size > 0) {
                std::vector<char> buffer(size);
                WideCharToMultiByte(CP_UTF8, 0, filename, -1, buffer.data(), size, nullptr, nullptr);
                std::string result = buffer.data();
                Logger::Info("Save location selected: " + result);
                return result;
            }
        } else {
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                Logger::Warning("GetSaveFileNameW() failed with error: " + std::to_string(err));
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
            ShellExecuteW(nullptr, L"open", L"explorer.exe", L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", nullptr, SW_SHOWNORMAL);
        } else {
            Logger::Debug("Opening explorer to: " + folderPath);
            // 转换路径到宽字符
            int len = MultiByteToWideChar(CP_UTF8, 0, folderPath.c_str(), -1, nullptr, 0);
            if (len > 0) {
                std::vector<wchar_t> wpath(len);
                MultiByteToWideChar(CP_UTF8, 0, folderPath.c_str(), -1, wpath.data(), len);
                // 打开指定文件夹
                ShellExecuteW(nullptr, L"open", L"explorer.exe", wpath.data(), nullptr, SW_SHOWNORMAL);
            }
        }
#endif
    } catch (const std::exception& e) {
        Logger::Error("Exception in OpenInExplorer(): " + std::string(e.what()));
    } catch (...) {
        Logger::Error("Unknown exception in OpenInExplorer()");
    }
}
