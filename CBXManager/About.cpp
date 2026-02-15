//==============================================================================
// About Dialog Implementation with Hardware Capabilities
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "stdafx.h"
#include "About.h"
#include <sstream>
#include <iomanip>
#include <intrin.h>
#include <dxgi.h>

#pragma comment(lib, "dxgi.lib")

void CAboutDlg::PopulateAboutInfo() {
	HWND hEdit = GetDlgItem(IDC_SORT_DESC);
	if (!hEdit) {
		return;
	}
	
	std::wostringstream oss;
	
	// Application version
	oss << L"DarkThumbs v6.2.0\r\n";
	oss << L"Build: " << __DATE__ << L" " << __TIME__ << L"\r\n";
	oss << L"\r\n";
	
	// Decoder capabilities
	oss << L"Engine Decoders: 21\r\n";
	oss << L"Registered Extensions: 150+\r\n";
	oss << L"\r\n";
	
	// Hardware information - detect directly
	try {
		oss << L"=== Hardware ===\r\n";
		
		// CPU info - detect manually
		int cpuInfo[4];
		char brandString[49] = {0};
		
		// Get brand string
		__cpuid(cpuInfo, 0x80000002);
		memcpy(brandString, cpuInfo, 16);
		__cpuid(cpuInfo, 0x80000003);
		memcpy(brandString + 16, cpuInfo, 16);
		__cpuid(cpuInfo, 0x80000004);
		memcpy(brandString + 32, cpuInfo, 16);
		
		// Trim whitespace
		std::string cpuBrand = brandString;
		size_t start = cpuBrand.find_first_not_of(" \t");
		size_t end = cpuBrand.find_last_not_of(" \t");
		if (start != std::string::npos && end != std::string::npos) {
			cpuBrand = cpuBrand.substr(start, end - start + 1);
		}
		
		oss << L"CPU: " << std::wstring(cpuBrand.begin(), cpuBrand.end()) << L"\r\n";
		
		// Core count
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		oss << L"Cores: " << sysInfo.dwNumberOfProcessors << L" logical\r\n";
		
		// SIMD capabilities
		__cpuid(cpuInfo, 7);
		bool hasAVX512 = (cpuInfo[1] & (1 << 16)) != 0;
		bool hasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;
		
		__cpuid(cpuInfo, 1);
		bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;
		bool hasSSE42 = (cpuInfo[2] & (1 << 20)) != 0;
		bool hasAESNI = (cpuInfo[2] & (1 << 25)) != 0;
		bool hasFMA = (cpuInfo[2] & (1 << 12)) != 0;
		
		oss << L"SIMD: ";
		if (hasAVX512) oss << L"AVX-512";
		else if (hasAVX2) oss << L"AVX2";
		else if (hasAVX) oss << L"AVX";
		else if (hasSSE42) oss << L"SSE4.2";
		else oss << L"SSE";
		
		if (hasAESNI) oss << L" + AES-NI";
		if (hasFMA) oss << L" + FMA";
		oss << L"\r\n";
		
		// GPU info using DXGI
		IDXGIFactory* factory = nullptr;
		if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
			IDXGIAdapter* adapter = nullptr;
			UINT adapterIdx = 0;
			
			oss << L"\r\n";
			while (factory->EnumAdapters(adapterIdx++, &adapter) != DXGI_ERROR_NOT_FOUND) {
				DXGI_ADAPTER_DESC desc;
				if (SUCCEEDED(adapter->GetDesc(&desc))) {
					oss << L"GPU" << adapterIdx << L": " << desc.Description;
					
					LARGE_INTEGER vram;
					vram.QuadPart = desc.DedicatedVideoMemory / (1024 * 1024);
					if (vram.QuadPart > 0) {
						oss << L" (" << vram.QuadPart << L" MB)";
					}
					oss << L"\r\n";
				}
				adapter->Release();
			}
			factory->Release();
		}
		
		// System memory
		MEMORYSTATUSEX memStatus;
		memStatus.dwLength = sizeof(memStatus);
		if (GlobalMemoryStatusEx(&memStatus)) {
			oss << L"\r\nRAM: " << (memStatus.ullTotalPhys / (1024 * 1024)) << L" MB";
			oss << L" (" << (memStatus.ullAvailPhys / (1024 * 1024)) << L" MB available)";
		}
		oss << L"\r\n";
		
		// Active optimizations
		oss << L"\r\n=== Active Optimizations ===\r\n";
		oss << L"- SIMD-accelerated scaling\r\n";
		oss << L"- GPU-accelerated rendering\r\n";
		oss << L"- Parallel decode (multi-threaded)\r\n";
		oss << L"- Smart thumbnail cache\r\n";
		if (hasAESNI) {
			oss << L"- Hardware AES encryption\r\n";
		}
		
	} catch (...) {
		// If hardware detection fails, show basic info
		oss << L"Hardware detection unavailable\r\n";
	}
	
	// Set the text
	::SetWindowTextW(hEdit, oss.str().c_str());
}
