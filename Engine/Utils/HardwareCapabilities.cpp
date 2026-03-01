//==============================================================================
// ExplorerLens Engine - Hardware Capabilities Implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "HardwareCapabilities.h"
#include <windows.h>
#include <intrin.h>
#include <dxgi1_4.h>
#include <d3d11.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// CPUCapabilities Implementation
//==============================================================================

std::string CPUCapabilities::GetBestSIMD() const {
    if (hasAVX512F) return "AVX-512";
    if (hasAVX2) return "AVX2";
    if (hasAVX) return "AVX";
    if (hasSSE42) return "SSE4.2";
    if (hasSSE41) return "SSE4.1";
    if (hasSSSE3) return "SSSE3";
    if (hasSSE3) return "SSE3";
    if (hasSSE2) return "SSE2";
    if (hasSSE) return "SSE";
    if (hasMMX) return "MMX";
    return "x87";
}

std::string CPUCapabilities::GetSummary() const {
    std::ostringstream oss;
    oss << brandString << " (" << physicalCores << "C/" << logicalCores << "T)";
    oss << " - " << GetBestSIMD();
    if (hasAESNI) oss << " + AES-NI";
    if (hasFMA) oss << " + FMA";
    return oss.str();
}

//==============================================================================
// GPUInfo Implementation
//==============================================================================

GPUVendor GPUInfo::GetVendorFromID(uint32_t vendorID) {
    switch (vendorID) {
    case 0x8086: return GPUVendor::Intel;
    case 0x10DE: return GPUVendor::NVIDIA;
    case 0x1002: return GPUVendor::AMD;
    case 0x1414: return GPUVendor::Microsoft;
    default: return GPUVendor::Unknown;
    }
}

std::string GPUInfo::GetSummary() const {
    std::ostringstream oss;
    oss << name;
    if (dedicatedMemoryMB > 0) {
        oss << " (" << dedicatedMemoryMB << " MB";
        if (sharedMemoryMB > 0) {
            oss << " + " << sharedMemoryMB << " MB shared";
        }
        oss << ")";
    }
    oss << " - D3D";
    if (supportsD3D12) oss << "12";
    else if (supportsD3D11) oss << "11";
    else oss << "?";
    return oss.str();
}

//==============================================================================
// HardwareCapabilities Implementation
//==============================================================================

HardwareCapabilities& HardwareCapabilities::Get() {
    static HardwareCapabilities instance;
    return instance;
}

HardwareCapabilities::HardwareCapabilities()
    : m_totalMemoryMB(0) {
    DetectCPU();
    DetectGPUs();
    DetectMemory();
}

void HardwareCapabilities::CPUID(int function, int subfunction, int* regs) {
    __cpuidex(regs, function, subfunction);
}

uint64_t HardwareCapabilities::XGETBV(uint32_t xcr) {
    return _xgetbv(xcr);
}

void HardwareCapabilities::DetectCPU() {
    int regs[4] = { 0 };

    // Get vendor string
    CPUID(0, 0, regs);
    int max_function = regs[0];
    char vendor[13] = { 0 };
    *reinterpret_cast<int*>(vendor) = regs[1];
    *reinterpret_cast<int*>(vendor + 4) = regs[3];
    *reinterpret_cast<int*>(vendor + 8) = regs[2];
    m_cpu.vendor = vendor;

    // Get brand string
    char brand[49] = { 0 };
    for (int i = 0; i < 3; i++) {
        CPUID(0x80000002 + i, 0, regs);
        memcpy(brand + i * 16, regs, 16);
    }
    m_cpu.brandString = brand;

    // Trim whitespace from brand string
    size_t start = m_cpu.brandString.find_first_not_of(" \t");
    size_t end = m_cpu.brandString.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos) {
        m_cpu.brandString = m_cpu.brandString.substr(start, end - start + 1);
    }

    // Get family/model/stepping
    if (max_function >= 1) {
        CPUID(1, 0, regs);
        m_cpu.stepping = regs[0] & 0xF;
        m_cpu.model = (regs[0] >> 4) & 0xF;
        m_cpu.family = (regs[0] >> 8) & 0xF;

        // Extended model/family
        if (m_cpu.family == 0xF) {
            m_cpu.family += (regs[0] >> 20) & 0xFF;
        }
        if (m_cpu.family == 0x6 || m_cpu.family == 0xF) {
            m_cpu.model += ((regs[0] >> 16) & 0xF) << 4;
        }

        // Feature flags - EDX register
        m_cpu.hasMMX = (regs[3] & (1 << 23)) != 0;
        m_cpu.hasSSE = (regs[3] & (1 << 25)) != 0;
        m_cpu.hasSSE2 = (regs[3] & (1 << 26)) != 0;
        m_cpu.hasCLFLUSH = (regs[3] & (1 << 19)) != 0;

        // Feature flags - ECX register
        m_cpu.hasSSE3 = (regs[2] & (1 << 0)) != 0;
        m_cpu.hasSSSE3 = (regs[2] & (1 << 9)) != 0;
        m_cpu.hasFMA = (regs[2] & (1 << 12)) != 0;
        m_cpu.hasSSE41 = (regs[2] & (1 << 19)) != 0;
        m_cpu.hasSSE42 = (regs[2] & (1 << 20)) != 0;
        m_cpu.hasPOPCNT = (regs[2] & (1 << 23)) != 0;
        m_cpu.hasAESNI = (regs[2] & (1 << 25)) != 0;
        m_cpu.hasAVX = (regs[2] & (1 << 28)) != 0;
        m_cpu.hasF16C = (regs[2] & (1 << 29)) != 0;
        m_cpu.hasRDRAND = (regs[2] & (1 << 30)) != 0;

        // Check OS support for AVX via XGETBV
        bool osSupportsAVX = false;
        if (m_cpu.hasAVX) {
            uint64_t xcr0 = XGETBV(0);
            osSupportsAVX = (xcr0 & 0x6) == 0x6; // XMM and YMM state
        }
        m_cpu.hasAVX = m_cpu.hasAVX && osSupportsAVX;
    }

    // Extended features (function 7)
    if (max_function >= 7) {
        CPUID(7, 0, regs);

        // EBX register
        m_cpu.hasBMI1 = (regs[1] & (1 << 3)) != 0;
        m_cpu.hasAVX2 = (regs[1] & (1 << 5)) != 0;
        m_cpu.hasBMI2 = (regs[1] & (1 << 8)) != 0;
        m_cpu.hasAVX512F = (regs[1] & (1 << 16)) != 0;
        m_cpu.hasAVX512DQ = (regs[1] & (1 << 17)) != 0;
        m_cpu.hasRDSEED = (regs[1] & (1 << 18)) != 0;
        m_cpu.hasADX = (regs[1] & (1 << 19)) != 0;
        m_cpu.hasAVX512IFMA = (regs[1] & (1 << 21)) != 0;
        m_cpu.hasCLFLUSHOPT = (regs[1] & (1 << 23)) != 0;
        m_cpu.hasCLWB = (regs[1] & (1 << 24)) != 0;
        m_cpu.hasAVX512PF = (regs[1] & (1 << 26)) != 0;
        m_cpu.hasAVX512ER = (regs[1] & (1 << 27)) != 0;
        m_cpu.hasAVX512CD = (regs[1] & (1 << 28)) != 0;
        m_cpu.hasSHA = (regs[1] & (1 << 29)) != 0;
        m_cpu.hasAVX512BW = (regs[1] & (1 << 30)) != 0;
        m_cpu.hasAVX512VL = (regs[1] & (1u << 31)) != 0;

        // ECX register
        m_cpu.hasAVX512VBMI = (regs[2] & (1 << 1)) != 0;
        m_cpu.hasAVX512VBMI2 = (regs[2] & (1 << 6)) != 0;
        m_cpu.hasAVX512VNNI = (regs[2] & (1 << 11)) != 0;
        m_cpu.hasAVX512BITALG = (regs[2] & (1 << 12)) != 0;
        m_cpu.hasAVX512VPOPCNTDQ = (regs[2] & (1 << 14)) != 0;

        // Check OS support for AVX-512 via XGETBV
        if (m_cpu.hasAVX512F) {
            uint64_t xcr0 = XGETBV(0);
            bool osSupportsAVX512 = (xcr0 & 0xE6) == 0xE6; // XMM, YMM, ZMM state
            if (!osSupportsAVX512) {
                // OS doesn't support AVX-512, disable all AVX-512 features
                m_cpu.hasAVX512F = false;
                m_cpu.hasAVX512DQ = false;
                m_cpu.hasAVX512IFMA = false;
                m_cpu.hasAVX512PF = false;
                m_cpu.hasAVX512ER = false;
                m_cpu.hasAVX512CD = false;
                m_cpu.hasAVX512BW = false;
                m_cpu.hasAVX512VL = false;
                m_cpu.hasAVX512VBMI = false;
                m_cpu.hasAVX512VBMI2 = false;
                m_cpu.hasAVX512VNNI = false;
                m_cpu.hasAVX512BITALG = false;
                m_cpu.hasAVX512VPOPCNTDQ = false;
            }
        }
    }

    // Extended processor info (AMD-specific features)
    CPUID(0x80000000, 0, regs);
    int max_ext_function = regs[0];

    if (max_ext_function >= 0x80000001) {
        CPUID(0x80000001, 0, regs);

        // ECX register
        m_cpu.hasLZCNT = (regs[2] & (1 << 5)) != 0;
        m_cpu.hasFMA4 = (regs[2] & (1 << 16)) != 0;
        m_cpu.hasPREFETCHWT1 = (regs[2] & (1 << 8)) != 0;
    }

    // Cache information
    if (max_ext_function >= 0x80000006) {
        CPUID(0x80000006, 0, regs);
        m_cpu.cacheLineSize = regs[2] & 0xFF;
        m_cpu.l2CacheKB = (regs[2] >> 16) & 0xFFFF;
        m_cpu.l3CacheKB = ((regs[3] >> 18) & 0x3FFF) * 512; // In 512KB units
    }

    // Core count
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_cpu.logicalCores = sysInfo.dwNumberOfProcessors;

    // Physical core count (approximate)
    m_cpu.physicalCores = m_cpu.logicalCores;
    if (max_function >= 0xB) {
        CPUID(0xB, 1, regs);
        if (regs[1] != 0) {
            m_cpu.physicalCores = regs[1] & 0xFFFF;
        }
    }

    // Default cache line size if not detected
    if (m_cpu.cacheLineSize == 0) {
        m_cpu.cacheLineSize = 64; // Common default
    }
}

void HardwareCapabilities::DetectGPUs() {
    m_gpus.clear();

    ComPtr<IDXGIFactory4> factory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        return;
    }

    UINT adapterIndex = 0;
    ComPtr<IDXGIAdapter1> adapter;

    while (factory->EnumAdapters1(adapterIndex++, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        if (SUCCEEDED(adapter->GetDesc1(&desc))) {
            // Skip software adapters
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                adapter.Reset();
                continue;
            }

            GPUInfo info;

            // Convert wide string to narrow
            char nameBuffer[256];
            WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                nameBuffer, sizeof(nameBuffer), nullptr, nullptr);
            info.name = nameBuffer;

            info.dedicatedMemoryMB = desc.DedicatedVideoMemory / (1024 * 1024);
            info.sharedMemoryMB = desc.SharedSystemMemory / (1024 * 1024);
            info.vendorID = desc.VendorId;
            info.deviceID = desc.DeviceId;
            info.vendor = GPUInfo::GetVendorFromID(desc.VendorId);

            // Test D3D11 support
            ComPtr<ID3D11Device> d3d11Device;
            D3D_FEATURE_LEVEL featureLevel;
            hr = D3D11CreateDevice(
                adapter.Get(),
                D3D_DRIVER_TYPE_UNKNOWN,
                nullptr,
                0,
                nullptr,
                0,
                D3D11_SDK_VERSION,
                &d3d11Device,
                &featureLevel,
                nullptr
            );
            info.supportsD3D11 = SUCCEEDED(hr);
            if (info.supportsD3D11) {
                info.featureLevel = static_cast<uint32_t>(featureLevel);
            }

            // Test D3D12 support
            hr = D3D12CreateDevice(
                adapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                _uuidof(ID3D12Device),
                nullptr
            );
            info.supportsD3D12 = SUCCEEDED(hr);

            // Query compute unit count via D3D12 if available
            info.supportsCompute = info.supportsD3D11;
            info.computeUnits = 0;
            if (info.supportsD3D12) {
                ComPtr<ID3D12Device> d3d12Device;
                HRESULT hrDev = D3D12CreateDevice(
                    adapter.Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    IID_PPV_ARGS(&d3d12Device)
                );
                if (SUCCEEDED(hrDev) && d3d12Device) {
                    D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
                    hrDev = d3d12Device->CheckFeatureSupport(
                        D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
                    if (SUCCEEDED(hrDev)) {
                        // Use resource binding tier as a proxy for GPU capability class
                        // Tier 3 = high-end (64+ CUs), Tier 2 = mid (16-64), Tier 1 = low (<16)
                        switch (options.ResourceBindingTier) {
                        case D3D12_RESOURCE_BINDING_TIER_3: info.computeUnits = 64; break;
                        case D3D12_RESOURCE_BINDING_TIER_2: info.computeUnits = 32; break;
                        default: info.computeUnits = 8; break;
                        }
                    }
                    info.supportsCompute = true;
                }
            }

            m_gpus.push_back(info);
        }

        adapter.Reset();
    }
}

void HardwareCapabilities::DetectMemory() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        m_totalMemoryMB = memStatus.ullTotalPhys / (1024 * 1024);
    }
}

uint64_t HardwareCapabilities::GetAvailableMemoryMB() const {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        return memStatus.ullAvailPhys / (1024 * 1024);
    }
    return 0;
}

const GPUInfo* HardwareCapabilities::GetPrimaryGPU() const {
    return m_gpus.empty() ? nullptr : &m_gpus[0];
}

const GPUInfo* HardwareCapabilities::GetPreferredGPU() const {
    // Prefer discrete GPU over integrated
    for (const auto& gpu : m_gpus) {
        if (gpu.vendor == GPUVendor::NVIDIA || gpu.vendor == GPUVendor::AMD) {
            if (gpu.dedicatedMemoryMB > 512) { // At least 512MB VRAM
                return &gpu;
            }
        }
    }

    // Fall back to primary
    return GetPrimaryGPU();
}

std::string HardwareCapabilities::GetFullReport() const {
    std::ostringstream oss;

    oss << "=== ExplorerLens Hardware Report ===\n\n";

    // CPU Information
    oss << "CPU:\n";
    oss << " Vendor: " << m_cpu.vendor << "\n";
    oss << " Model: " << m_cpu.brandString << "\n";
    oss << " Cores: " << m_cpu.physicalCores << " physical, "
        << m_cpu.logicalCores << " logical\n";
    oss << " Family/Model/Stepping: " << m_cpu.family << "/"
        << m_cpu.model << "/" << m_cpu.stepping << "\n";
    oss << " Cache: L1=" << m_cpu.l1CacheKB << "KB, L2="
        << m_cpu.l2CacheKB << "KB, L3=" << m_cpu.l3CacheKB << "KB\n";
    oss << " Cache Line: " << m_cpu.cacheLineSize << " bytes\n\n";

    oss << " SIMD Instructions:\n";
    oss << " Best Available: " << m_cpu.GetBestSIMD() << "\n";
    oss << " MMX: " << (m_cpu.hasMMX ? "Yes" : "No") << "\n";
    oss << " SSE: " << (m_cpu.hasSSE ? "Yes" : "No") << "\n";
    oss << " SSE2: " << (m_cpu.hasSSE2 ? "Yes" : "No") << "\n";
    oss << " SSE3: " << (m_cpu.hasSSE3 ? "Yes" : "No") << "\n";
    oss << " SSSE3: " << (m_cpu.hasSSSE3 ? "Yes" : "No") << "\n";
    oss << " SSE4.1: " << (m_cpu.hasSSE41 ? "Yes" : "No") << "\n";
    oss << " SSE4.2: " << (m_cpu.hasSSE42 ? "Yes" : "No") << "\n";
    oss << " AVX: " << (m_cpu.hasAVX ? "Yes" : "No") << "\n";
    oss << " AVX2: " << (m_cpu.hasAVX2 ? "Yes" : "No") << "\n";
    oss << " AVX-512F: " << (m_cpu.hasAVX512F ? "Yes" : "No") << "\n";
    if (m_cpu.hasAVX512F) {
        oss << " AVX-512DQ: " << (m_cpu.hasAVX512DQ ? "Yes" : "No") << "\n";
        oss << " AVX-512BW: " << (m_cpu.hasAVX512BW ? "Yes" : "No") << "\n";
        oss << " AVX-512VL: " << (m_cpu.hasAVX512VL ? "Yes" : "No") << "\n";
        oss << " AVX-512VNNI: " << (m_cpu.hasAVX512VNNI ? "Yes" : "No") << "\n";
    }

    oss << "\n Other Features:\n";
    oss << " AES-NI: " << (m_cpu.hasAESNI ? "Yes" : "No") << "\n";
    oss << " SHA: " << (m_cpu.hasSHA ? "Yes" : "No") << "\n";
    oss << " FMA: " << (m_cpu.hasFMA ? "Yes" : "No") << "\n";
    oss << " F16C: " << (m_cpu.hasF16C ? "Yes" : "No") << "\n";
    oss << " BMI1/2: " << (m_cpu.hasBMI1 ? "Yes" : "No")
        << "/" << (m_cpu.hasBMI2 ? "Yes" : "No") << "\n";
    oss << " POPCNT: " << (m_cpu.hasPOPCNT ? "Yes" : "No") << "\n";
    oss << " RDRAND: " << (m_cpu.hasRDRAND ? "Yes" : "No") << "\n";

    // GPU Information
    oss << "\nGPUs (" << m_gpus.size() << " detected):\n";
    for (size_t i = 0; i < m_gpus.size(); i++) {
        const auto& gpu = m_gpus[i];
        oss << " [" << i << "] " << gpu.name << "\n";
        oss << " Vendor: ";
        switch (gpu.vendor) {
        case GPUVendor::Intel: oss << "Intel"; break;
        case GPUVendor::NVIDIA: oss << "NVIDIA"; break;
        case GPUVendor::AMD: oss << "AMD"; break;
        case GPUVendor::Microsoft: oss << "Microsoft (Software)"; break;
        default: oss << "Unknown"; break;
        }
        oss << " (0x" << std::hex << std::setw(4) << std::setfill('0')
            << gpu.vendorID << ":" << gpu.deviceID << std::dec << ")\n";
        oss << " VRAM: " << gpu.dedicatedMemoryMB << " MB dedicated";
        if (gpu.sharedMemoryMB > 0) {
            oss << " + " << gpu.sharedMemoryMB << " MB shared";
        }
        oss << "\n";
        oss << " DirectX: ";
        if (gpu.supportsD3D12) oss << "D3D12 ";
        if (gpu.supportsD3D11) oss << "D3D11 ";
        oss << "(Feature Level " << std::hex << gpu.featureLevel << std::dec << ")\n";
        oss << " Compute: " << (gpu.supportsCompute ? "Yes" : "No") << "\n";
    }

    // Memory
    oss << "\nSystem Memory:\n";
    oss << " Total: " << m_totalMemoryMB << " MB\n";
    oss << " Available: " << GetAvailableMemoryMB() << " MB\n";

    return oss.str();
}

std::string HardwareCapabilities::GetSummary() const {
    std::ostringstream oss;
    oss << m_cpu.GetSummary();

    const GPUInfo* primaryGPU = GetPrimaryGPU();
    if (primaryGPU) {
        oss << " | GPU: " << primaryGPU->GetSummary();
    }

    oss << " | RAM: " << m_totalMemoryMB << " MB";
    return oss.str();
}

bool HardwareCapabilities::HasFeature(const char* feature) const {
    std::string feat = feature;

    // CPU features
    if (feat == "AVX512" || feat == "AVX-512") return m_cpu.hasAVX512F;
    if (feat == "AVX2") return m_cpu.hasAVX2;
    if (feat == "AVX") return m_cpu.hasAVX;
    if (feat == "SSE4.2") return m_cpu.hasSSE42;
    if (feat == "SSE4.1") return m_cpu.hasSSE41;
    if (feat == "SSSE3") return m_cpu.hasSSSE3;
    if (feat == "SSE3") return m_cpu.hasSSE3;
    if (feat == "SSE2") return m_cpu.hasSSE2;
    if (feat == "SSE") return m_cpu.hasSSE;
    if (feat == "MMX") return m_cpu.hasMMX;
    if (feat == "AES" || feat == "AES-NI") return m_cpu.hasAESNI;
    if (feat == "SHA") return m_cpu.hasSHA;
    if (feat == "FMA") return m_cpu.hasFMA;

    // GPU features
    if (feat == "D3D11") {
        for (const auto& gpu : m_gpus) {
            if (gpu.supportsD3D11) return true;
        }
    }
    if (feat == "D3D12") {
        for (const auto& gpu : m_gpus) {
            if (gpu.supportsD3D12) return true;
        }
    }

    return false;
}

void HardwareCapabilities::RefreshGPUs() {
    DetectGPUs();
}

} // namespace Engine
} // namespace ExplorerLens
