// GPUValidator.cpp - GPU Detection and Validation Tool
// DarkThumbs v5.2.0 - Multi-GPU Testing
// Copyright (c) 2025 DarkThumbs Project
//
// Tests GPU detection, capabilities, and fallback mechanisms
// Validates behavior across Intel, NVIDIA, AMD, and WARP renderers

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

struct GPUInfo {
    std::wstring name;
    UINT vendorId;
    std::wstring vendorName;
    SIZE_T dedicatedVideoMemory;
    SIZE_T dedicatedSystemMemory;
    SIZE_T sharedSystemMemory;
    D3D_FEATURE_LEVEL featureLevel;
    bool isHardware;
    LUID adapterLuid;
};

std::wstring GetVendorName(UINT vendorId) {
    switch (vendorId) {
        case 0x8086: return L"Intel";
        case 0x10DE: return L"NVIDIA";
        case 0x1002: return L"AMD";
        case 0x1414: return L"Microsoft (WARP)";
        default: return L"Unknown";
    }
}

std::wstring FormatBytes(SIZE_T bytes) {
    if (bytes >= 1024ULL * 1024 * 1024) {
        return std::to_wstring(bytes / (1024.0 * 1024 * 1024)) + L" GB";
    } else if (bytes >= 1024ULL * 1024) {
        return std::to_wstring(bytes / (1024.0 * 1024)) + L" MB";
    } else if (bytes >= 1024) {
        return std::to_wstring(bytes / 1024.0) + L" KB";
    }
    return std::to_wstring(bytes) + L" bytes";
}

std::wstring GetFeatureLevelName(D3D_FEATURE_LEVEL level) {
    switch (level) {
        case D3D_FEATURE_LEVEL_12_1: return L"DirectX 12.1";
        case D3D_FEATURE_LEVEL_12_0: return L"DirectX 12.0";
        case D3D_FEATURE_LEVEL_11_1: return L"DirectX 11.1";
        case D3D_FEATURE_LEVEL_11_0: return L"DirectX 11.0";
        case D3D_FEATURE_LEVEL_10_1: return L"DirectX 10.1";
        case D3D_FEATURE_LEVEL_10_0: return L"DirectX 10.0";
        case D3D_FEATURE_LEVEL_9_3: return L"DirectX 9.3";
        case D3D_FEATURE_LEVEL_9_2: return L"DirectX 9.2";
        case D3D_FEATURE_LEVEL_9_1: return L"DirectX 9.1";
        default: return L"Unknown";
    }
}

bool EnumerateGPUs(std::vector<GPUInfo>& gpus) {
    ComPtr<IDXGIFactory1> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create DXGI factory: 0x" << std::hex << hr << std::endl;
        return false;
    }

    UINT adapterIndex = 0;
    ComPtr<IDXGIAdapter1> adapter;

    while (factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        GPUInfo info;
        info.name = desc.Description;
        info.vendorId = desc.VendorId;
        info.vendorName = GetVendorName(desc.VendorId);
        info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
        info.dedicatedSystemMemory = desc.DedicatedSystemMemory;
        info.sharedSystemMemory = desc.SharedSystemMemory;
        info.adapterLuid = desc.AdapterLuid;

        // Try to create device to get feature level
        ComPtr<ID3D11Device> device;
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL achievedLevel;

        hr = D3D11CreateDevice(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            0,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &device,
            &achievedLevel,
            nullptr
        );

        if (SUCCEEDED(hr)) {
            info.featureLevel = achievedLevel;
            info.isHardware = true;
        } else {
            info.featureLevel = D3D_FEATURE_LEVEL_10_0; // Unknown
            info.isHardware = false;
        }

        gpus.push_back(info);

        adapterIndex++;
        adapter.Reset();
    }

    return !gpus.empty();
}

bool TestWARPRenderer(GPUInfo& warpInfo) {
    ComPtr<ID3D11Device> device;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL achievedLevel;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_WARP,
        nullptr,
        0,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &device,
        &achievedLevel,
        nullptr
    );

    if (SUCCEEDED(hr)) {
        warpInfo.name = L"WARP Software Renderer";
        warpInfo.vendorId = 0x1414;
        warpInfo.vendorName = L"Microsoft";
        warpInfo.dedicatedVideoMemory = 0;
        warpInfo.dedicatedSystemMemory = 0;
        warpInfo.sharedSystemMemory = 0;
        warpInfo.featureLevel = achievedLevel;
        warpInfo.isHardware = false;
        return true;
    }

    return false;
}

bool TestComputeShaderSupport(const GPUInfo& gpu) {
    ComPtr<IDXGIFactory1> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    ComPtr<IDXGIAdapter1> adapter;
    UINT index = 0;
    while (factory->EnumAdapters1(index++, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.AdapterLuid.LowPart == gpu.adapterLuid.LowPart &&
            desc.AdapterLuid.HighPart == gpu.adapterLuid.HighPart) {
            
            ComPtr<ID3D11Device> device;
            D3D_FEATURE_LEVEL featureLevel;
            
            HRESULT hr = D3D11CreateDevice(
                adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                0,
                nullptr,
                0,
                D3D11_SDK_VERSION,
                &device,
                &featureLevel,
                nullptr
            );

            if (SUCCEEDED(hr)) {
                return featureLevel >= D3D_FEATURE_LEVEL_11_0;
            }
        }
        adapter.Reset();
    }

    return false;
}

void PrintGPUInfo(const GPUInfo& gpu, int index) {
    std::wcout << L"\n=================================================\n";
    std::wcout << L"GPU #" << index << L": " << gpu.name << L"\n";
    std::wcout << L"=================================================\n";
    std::wcout << L"Vendor:                " << gpu.vendorName << L" (0x" << std::hex << gpu.vendorId << std::dec << L")\n";
    std::wcout << L"Feature Level:         " << GetFeatureLevelName(gpu.featureLevel) << L"\n";
    std::wcout << L"Device Type:           " << (gpu.isHardware ? L"Hardware" : L"Software") << L"\n";
    
    if (gpu.dedicatedVideoMemory > 0) {
        std::wcout << L"Dedicated VRAM:        " << FormatBytes(gpu.dedicatedVideoMemory) << L"\n";
    }
    if (gpu.dedicatedSystemMemory > 0) {
        std::wcout << L"Dedicated System:      " << FormatBytes(gpu.dedicatedSystemMemory) << L"\n";
    }
    if (gpu.sharedSystemMemory > 0) {
        std::wcout << L"Shared System:         " << FormatBytes(gpu.sharedSystemMemory) << L"\n";
    }

    // Compute shader support
    bool csSupport = gpu.featureLevel >= D3D_FEATURE_LEVEL_11_0;
    std::wcout << L"Compute Shaders:       " << (csSupport ? L"✓ Supported" : L"✗ Not Supported") << L"\n";

    // DarkThumbs compatibility
    bool compatible = gpu.featureLevel >= D3D_FEATURE_LEVEL_11_0;
    std::wcout << L"DarkThumbs GPU Mode:   " << (compatible ? L"✓ ENABLED" : L"✗ CPU Fallback") << L"\n";

    // Intel GPU type detection
    if (gpu.vendorId == 0x8086) {
        std::wstring gpuType = L"Unknown";
        if (gpu.name.find(L"Arc") != std::wstring::npos) {
            gpuType = L"Arc (Discrete)";
        } else if (gpu.name.find(L"Iris Xe") != std::wstring::npos || gpu.name.find(L"Xe") != std::wstring::npos) {
            gpuType = L"Iris Xe (High-Performance)";
        } else if (gpu.name.find(L"Iris") != std::wstring::npos) {
            gpuType = L"Iris/Iris Plus";
        } else if (gpu.name.find(L"UHD") != std::wstring::npos) {
            gpuType = L"UHD Graphics (Integrated)";
        } else if (gpu.name.find(L"HD") != std::wstring::npos) {
            gpuType = L"HD Graphics (Integrated)";
        }
        std::wcout << L"Intel GPU Type:        " << gpuType << L"\n";
    }
}

void PrintSummary(const std::vector<GPUInfo>& gpus, const GPUInfo* warp) {
    std::wcout << L"\n\n";
    std::wcout << L"=================================================\n";
    std::wcout << L"SUMMARY\n";
    std::wcout << L"=================================================\n";
    std::wcout << L"Total GPUs Found:      " << gpus.size() << L"\n";

    int compatibleCount = 0;
    int intelCount = 0;
    int nvidiaCount = 0;
    int amdCount = 0;

    for (const auto& gpu : gpus) {
        if (gpu.featureLevel >= D3D_FEATURE_LEVEL_11_0) {
            compatibleCount++;
        }
        if (gpu.vendorId == 0x8086) intelCount++;
        if (gpu.vendorId == 0x10DE) nvidiaCount++;
        if (gpu.vendorId == 0x1002) amdCount++;
    }

    std::wcout << L"DarkThumbs Compatible: " << compatibleCount << L" GPU(s)\n";
    if (intelCount > 0) std::wcout << L"Intel GPUs:            " << intelCount << L"\n";
    if (nvidiaCount > 0) std::wcout << L"NVIDIA GPUs:           " << nvidiaCount << L"\n";
    if (amdCount > 0) std::wcout << L"AMD GPUs:              " << amdCount << L"\n";
    
    if (warp) {
        std::wcout << L"WARP Fallback:         ✓ Available (" << GetFeatureLevelName(warp->featureLevel) << L")\n";
    } else {
        std::wcout << L"WARP Fallback:         ✗ Not Available\n";
    }

    std::wcout << L"\n";
    if (compatibleCount > 0) {
        std::wcout << L"✓ DarkThumbs will use GPU acceleration\n";
        std::wcout << L"✓ Expected speedup: 4-10x depending on GPU\n";
    } else if (warp) {
        std::wcout << L"⚠ DarkThumbs will use WARP software renderer\n";
        std::wcout << L"⚠ Performance may be similar to CPU fallback\n";
    } else {
        std::wcout << L"✗ DarkThumbs will use CPU-only rendering\n";
        std::wcout << L"✗ No GPU acceleration available\n";
    }
}

int main(int argc, char* argv[]) {
    std::wcout << L"=================================================\n";
    std::wcout << L"DarkThumbs GPU Validator v5.2.0\n";
    std::wcout << L"Multi-GPU Detection and Validation Tool\n";
    std::wcout << L"=================================================\n\n";

    // Enumerate all GPUs
    std::vector<GPUInfo> gpus;
    if (!EnumerateGPUs(gpus)) {
        std::wcerr << L"ERROR: Failed to enumerate GPUs\n";
        return 1;
    }

    // Print info for each GPU
    for (size_t i = 0; i < gpus.size(); i++) {
        PrintGPUInfo(gpus[i], static_cast<int>(i + 1));
    }

    // Test WARP renderer
    GPUInfo warpInfo;
    bool warpAvailable = TestWARPRenderer(warpInfo);
    if (warpAvailable) {
        PrintGPUInfo(warpInfo, static_cast<int>(gpus.size() + 1));
    }

    // Print summary
    PrintSummary(gpus, warpAvailable ? &warpInfo : nullptr);

    std::wcout << L"\n";
    std::wcout << L"Press Enter to exit...";
    std::cin.get();

    return 0;
}
