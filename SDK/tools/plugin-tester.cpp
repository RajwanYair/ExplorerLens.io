/******************************************************************************
 * ExplorerLens Plugin Tester
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Command-line tool for testing and validating plugins.
 *****************************************************************************/

#include "../../SDK/plugin_api.h"
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>

namespace fs = std::filesystem;

//============================================================================
// Plugin Loader
//============================================================================

class PluginTester {
public:
    PluginTester() = default;
    ~PluginTester() { UnloadPlugin(); }
    
    bool LoadPlugin(const fs::path& plugin_path) {
        plugin_path_ = plugin_path;
        
        // Load DLL
        module_ = LoadLibraryW(plugin_path.c_str());
        if (!module_) {
            std::wcerr << L"ERROR: Failed to load plugin DLL\n";
            std::wcerr << L"  Path: " << plugin_path << L"\n";
            std::wcerr << L"  Error code: " << GetLastError() << L"\n";
            return false;
        }
        
        // Get function pointers
        get_info_ = reinterpret_cast<decltype(get_info_)>(
            GetProcAddress(module_, "plugin_get_info"));
        init_ = reinterpret_cast<decltype(init_)>(
            GetProcAddress(module_, "plugin_init"));
        cleanup_ = reinterpret_cast<decltype(cleanup_)>(
            GetProcAddress(module_, "plugin_cleanup"));
        can_decode_ = reinterpret_cast<decltype(can_decode_)>(
            GetProcAddress(module_, "plugin_can_decode"));
        decode_ = reinterpret_cast<decltype(decode_)>(
            GetProcAddress(module_, "plugin_decode"));
        free_result_ = reinterpret_cast<decltype(free_result_)>(
            GetProcAddress(module_, "plugin_free_result"));
        
        // Validate required functions
        if (!get_info_ || !init_ || !cleanup_ || !can_decode_ || !decode_ || !free_result_) {
            std::cerr << "ERROR: Plugin missing required exports\n";
            std::cerr << "  plugin_get_info:   " << (get_info_ ? "OK" : "MISSING") << "\n";
            std::cerr << "  plugin_init:       " << (init_ ? "OK" : "MISSING") << "\n";
            std::cerr << "  plugin_cleanup:    " << (cleanup_ ? "OK" : "MISSING") << "\n";
            std::cerr << "  plugin_can_decode: " << (can_decode_ ? "OK" : "MISSING") << "\n";
            std::cerr << "  plugin_decode:     " << (decode_ ? "OK" : "MISSING") << "\n";
            std::cerr << "  plugin_free_result:" << (free_result_ ? "OK" : "MISSING") << "\n";
            UnloadPlugin();
            return false;
        }
        
        // Get plugin info
        info_ = get_info_();
        if (!info_) {
            std::cerr << "ERROR: plugin_get_info() returned NULL\n";
            UnloadPlugin();
            return false;
        }
        
        // Check API version
        uint32_t plugin_major = (info_->api_version >> 16) & 0xFFFF;
        uint32_t plugin_minor = info_->api_version & 0xFFFF;
        uint32_t host_major = (EXPLORERLENS_PLUGIN_API_VERSION >> 16) & 0xFFFF;
        uint32_t host_minor = EXPLORERLENS_PLUGIN_API_VERSION & 0xFFFF;
        
        std::cout << "Plugin API version: " << plugin_major << "." << plugin_minor << "\n";
        std::cout << "Host API version:   " << host_major << "." << host_minor << "\n";
        
        if (plugin_major != host_major) {
            std::cerr << "ERROR: Incompatible API version (major version mismatch)\n";
            UnloadPlugin();
            return false;
        }
        
        // Initialize plugin
        PluginAllocator allocator = {
            .alloc = &PluginTester::PluginAlloc,
            .free = &PluginTester::PluginFree,
            .user_data = this
        };
        
        auto result = init_(&allocator);
        if (result != PLUGIN_SUCCESS) {
            std::cerr << "ERROR: plugin_init() failed with code " << result << "\n";
            UnloadPlugin();
            return false;
        }
        
        initialized_ = true;
        return true;
    }
    
    void UnloadPlugin() {
        if (module_) {
            if (initialized_ && cleanup_) {
                cleanup_();
                initialized_ = false;
            }
            FreeLibrary(module_);
            module_ = nullptr;
        }
    }
    
    void PrintInfo() const {
        if (!info_) return;
        
        std::cout << "\n=== Plugin Information ===\n";
        std::cout << "Name:        " << (info_->plugin_name ? info_->plugin_name : "N/A") << "\n";
        std::cout << "Version:     " << (info_->plugin_version ? info_->plugin_version : "N/A") << "\n";
        std::cout << "Author:      " << (info_->plugin_author ? info_->plugin_author : "N/A") << "\n";
        std::cout << "Description: " << (info_->plugin_description ? info_->plugin_description : "N/A") << "\n";
        std::cout << "License:     " << (info_->plugin_license ? info_->plugin_license : "N/A") << "\n";
        
        std::cout << "\nCapabilities:\n";
        if (info_->capabilities & PLUGIN_CAP_STILL_IMAGE)  std::cout << "  - Still images\n";
        if (info_->capabilities & PLUGIN_CAP_ANIMATION)    std::cout << "  - Animations\n";
        if (info_->capabilities & PLUGIN_CAP_MULTIPAGE)    std::cout << "  - Multi-page\n";
        if (info_->capabilities & PLUGIN_CAP_PROGRESSIVE)  std::cout << "  - Progressive decoding\n";
        if (info_->capabilities & PLUGIN_CAP_METADATA)     std::cout << "  - Metadata extraction\n";
        if (info_->capabilities & PLUGIN_CAP_THUMBNAIL)    std::cout << "  - Embedded thumbnails\n";
        if (info_->capabilities & PLUGIN_CAP_GPU_DECODE)   std::cout << "  - GPU acceleration\n";
        if (info_->capabilities & PLUGIN_CAP_HDR)          std::cout << "  - HDR support\n";
        if (info_->capabilities & PLUGIN_CAP_ALPHA)        std::cout << "  - Alpha channel\n";
        
        std::cout << "\nSupported Extensions:\n";
        if (info_->supported_extensions) {
            for (size_t i = 0; info_->supported_extensions[i] != nullptr; ++i) {
                std::cout << "  - " << info_->supported_extensions[i] << "\n";
            }
        }
        
        std::cout << "\nSupported MIME Types:\n";
        if (info_->mime_types) {
            for (size_t i = 0; info_->mime_types[i] != nullptr; ++i) {
                std::cout << "  - " << info_->mime_types[i] << "\n";
            }
        }
        
        std::cout << "\nPerformance Hints:\n";
        std::cout << "  Max Threads:    " << (info_->max_threads ? std::to_string(info_->max_threads) : "Unlimited") << "\n";
        std::cout << "  Requires GPU:   " << (info_->requires_gpu ? "Yes" : "No") << "\n";
        std::cout << "  Background OK:  " << (info_->supports_background_loading ? "Yes" : "No") << "\n";
        std::cout << "\n";
    }
    
    bool TestDecode(const fs::path& test_file, uint32_t width = 256, uint32_t height = 256) {
        if (!initialized_ || !can_decode_ || !decode_) {
            std::cerr << "ERROR: Plugin not initialized\n";
            return false;
        }
        
        std::cout << "\n=== Testing Decode ===\n";
        std::wcout << L"File: " << test_file << L"\n";
        
        // Check if file exists
        if (!fs::exists(test_file)) {
            std::wcerr << L"ERROR: File not found: " << test_file << L"\n";
            return false;
        }
        
        std::string file_utf8 = test_file.u8string();
        
        // Test can_decode
        std::cout << "Testing plugin_can_decode()... ";
        bool can_decode = can_decode_(file_utf8.c_str(), nullptr, 0);
        std::cout << (can_decode ? "YES" : "NO") << "\n";
        
        if (!can_decode) {
            std::cout << "Plugin cannot decode this file\n";
            return false;
        }
        
        // Prepare decode request
        DecodeRequest request = {};
        request.file_path = file_utf8.c_str();
        request.data = nullptr;
        request.data_size = 0;
        request.target_width = width;
        request.target_height = height;
        request.output_format = PIXEL_FORMAT_BGRA32;
        request.preserve_aspect_ratio = true;
        request.high_quality = true;
        request.frame_index = 0;
        request.user_data = nullptr;
        
        DecodeResult result = {};
        
        // Measure decode time
        auto start = std::chrono::high_resolution_clock::now();
        
        std::cout << "Decoding... ";
        auto error = decode_(&request, &result, nullptr);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (error != PLUGIN_SUCCESS) {
            std::cerr << "\nERROR: Decode failed with code " << error << "\n";
            if (result.error_message) {
                std::cerr << "  Message: " << result.error_message << "\n";
            }
            return false;
        }
        
        std::cout << "SUCCESS (" << duration.count() << " ms)\n";
        
        // Print result info
        std::cout << "\nDecode Result:\n";
        std::cout << "  Dimensions: " << result.width << "x" << result.height << "\n";
        std::cout << "  Buffer:     " << result.buffer_size << " bytes\n";
        std::cout << "  Stride:     " << result.stride << " bytes/row\n";
        std::cout << "  Format:     ";
        switch (result.pixel_format) {
            case PIXEL_FORMAT_BGRA32: std::cout << "BGRA32\n"; break;
            case PIXEL_FORMAT_RGBA32: std::cout << "RGBA32\n"; break;
            case PIXEL_FORMAT_BGR24:  std::cout << "BGR24\n"; break;
            case PIXEL_FORMAT_RGB24:  std::cout << "RGB24\n"; break;
            default: std::cout << "Unknown\n"; break;
        }
        
        // Validate result
        if (!result.pixels || result.buffer_size == 0) {
            std::cerr << "ERROR: Invalid result (null buffer)\n";
            return false;
        }
        
        // Free result
        if (free_result_) {
            free_result_(&result);
            std::cout << "  Freed:      OK\n";
        }
        
        return true;
    }
    
private:
    static void* PluginAlloc(size_t size, void* user_data) {
        return malloc(size);
    }
    
    static void PluginFree(void* ptr, void* user_data) {
        free(ptr);
    }
    
    fs::path plugin_path_;
    HMODULE module_ = nullptr;
    const PluginInfo* info_ = nullptr;
    bool initialized_ = false;
    
    decltype(&plugin_get_info) get_info_ = nullptr;
    decltype(&plugin_init) init_ = nullptr;
    decltype(&plugin_cleanup) cleanup_ = nullptr;
    decltype(&plugin_can_decode) can_decode_ = nullptr;
    decltype(&plugin_decode) decode_ = nullptr;
    decltype(&plugin_free_result) free_result_ = nullptr;
};

//============================================================================
// Main
//============================================================================

void PrintUsage(const char* program) {
    std::cout << "\nExplorerLens Plugin Tester v1.0\n";
    std::cout << "==============================\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << program << " <plugin.dll> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --info              Show plugin information only\n";
    std::cout << "  --test <file>       Test decode on specified file\n";
    std::cout << "  --size <WxH>        Output size (default: 256x256)\n";
    std::cout << "  --help              Show this help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program << " webp-plugin.dll --info\n";
    std::cout << "  " << program << " webp-plugin.dll --test image.webp\n";
    std::cout << "  " << program << " webp-plugin.dll --test image.webp --size 512x512\n\n";
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        PrintUsage("plugin-tester.exe");
        return 1;
    }
    
    fs::path plugin_path = argv[1];
    bool show_info = false;
    bool test_decode = false;
    fs::path test_file;
    uint32_t width = 256;
    uint32_t height = 256;
    
    // Parse arguments
    for (int i = 2; i < argc; ++i) {
        std::wstring arg = argv[i];
        
        if (arg == L"--help") {
            PrintUsage("plugin-tester.exe");
            return 0;
        }
        else if (arg == L"--info") {
            show_info = true;
        }
        else if (arg == L"--test") {
            if (i + 1 < argc) {
                test_file = argv[++i];
                test_decode = true;
            }
        }
        else if (arg == L"--size") {
            if (i + 1 < argc) {
                std::wstring size_str = argv[++i];
                size_t x_pos = size_str.find(L'x');
                if (x_pos != std::wstring::npos) {
                    width = std::stoi(size_str.substr(0, x_pos));
                    height = std::stoi(size_str.substr(x_pos + 1));
                }
            }
        }
    }
    
    // Load plugin
    std::wcout << L"Loading plugin: " << plugin_path << L"\n";
    
    PluginTester tester;
    if (!tester.LoadPlugin(plugin_path)) {
        return 1;
    }
    
    std::cout << "Plugin loaded successfully!\n";
    
    // Show info if requested or no other actions
    if (show_info || !test_decode) {
        tester.PrintInfo();
    }
    
    // Test decode if requested
    if (test_decode) {
        if (!tester.TestDecode(test_file, width, height)) {
            return 1;
        }
    }
    
    std::cout << "\n=== All Tests Passed ===\n";
    return 0;
}


