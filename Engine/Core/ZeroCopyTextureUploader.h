// ZeroCopyTextureUploader.h — Direct-Mapped VRAM Upload for Thumbnail Textures
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a pool of upload heaps (D3D12 UPLOAD heap type) mapped persistently
// to CPU address space, enabling zero-copy transfer of decoded RGBA pixel data
// directly into GPU-accessible memory without intermediate staging copies.
//
#pragma once
#include <windows.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <cstdint>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace ExplorerLens { namespace Engine {

struct UploadSlot {
    void*   cpuPtr     = nullptr; // Persistently mapped write-combine memory
    size_t  capacity   = 0;       // Slot capacity in bytes
    size_t  used       = 0;
    bool    inUse      = false;
    ID3D12Resource* resource = nullptr; // D3D12 upload heap resource
};

struct ZeroCopyUploadStats {
    uint64_t totalBytesUploaded = 0;
    uint64_t uploadCount        = 0;
    double   avgUploadUs        = 0.0; // Microseconds
    size_t   peakUsageBytes     = 0;
    int      poolSlots          = 0;
};

class ZeroCopyTextureUploader {
public:
    static constexpr size_t DEFAULT_SLOT_BYTES = 4 * 1024 * 1024; // 4 MB per slot
    static constexpr int    DEFAULT_POOL_SIZE  = 8;

    ~ZeroCopyTextureUploader() { Destroy(); }

    bool Initialize(ID3D12Device* device,
                    int   poolSize  = DEFAULT_POOL_SIZE,
                    size_t slotBytes = DEFAULT_SLOT_BYTES) {
        if (!device) return false;
        m_device = device;

        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        rd.Width            = static_cast<UINT64>(slotBytes);
        rd.Height           = 1;
        rd.DepthOrArraySize = 1;
        rd.MipLevels        = 1;
        rd.SampleDesc.Count = 1;
        rd.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        for (int i = 0; i < poolSize; ++i) {
            UploadSlot slot{};
            slot.capacity = slotBytes;
            HRESULT hr = device->CreateCommittedResource(
                &hp, D3D12_HEAP_FLAG_NONE, &rd,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr, IID_PPV_ARGS(&slot.resource));
            if (FAILED(hr)) continue;
            // Persistently map the entire buffer (write-combine)
            D3D12_RANGE empty = {};
            hr = slot.resource->Map(0, &empty, &slot.cpuPtr);
            if (FAILED(hr)) { slot.resource->Release(); continue; }
            m_pool.push_back(slot);
        }
        return !m_pool.empty();
    }

    // Upload pixelData to GPU upload heap; returns CPU pointer to mapped region
    // Caller must coordinate GPU fence before releasing the slot
    bool Upload(const uint8_t* pixelData, size_t bytes,
                UploadSlot** outSlot) {
        std::lock_guard<std::mutex> lk(m_mtx);
        UploadSlot* slot = AcquireSlot(bytes);
        if (!slot) return false;

        LARGE_INTEGER t0, t1, freq;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&t0);

        // Write directly to persistently mapped write-combine memory
        memcpy(slot->cpuPtr, pixelData, bytes);
        slot->used = bytes;

        QueryPerformanceCounter(&t1);
        double us = (t1.QuadPart - t0.QuadPart) * 1e6 / freq.QuadPart;

        m_stats.totalBytesUploaded += bytes;
        m_stats.uploadCount++;
        double prev = m_stats.avgUploadUs;
        m_stats.avgUploadUs = prev + (us - prev) / m_stats.uploadCount;
        if (bytes > m_stats.peakUsageBytes) m_stats.peakUsageBytes = bytes;

        *outSlot = slot;
        return true;
    }

    // Return a slot to the pool after the GPU has consumed it
    void Release(UploadSlot* slot) {
        std::lock_guard<std::mutex> lk(m_mtx);
        if (slot) { slot->inUse = false; slot->used = 0; }
    }

    const ZeroCopyUploadStats& Stats() const { return m_stats; }

    void Destroy() {
        std::lock_guard<std::mutex> lk(m_mtx);
        for (auto& s : m_pool) {
            if (s.resource) {
                D3D12_RANGE empty = {};
                s.resource->Unmap(0, &empty);
                s.resource->Release();
                s.resource = nullptr;
            }
        }
        m_pool.clear();
    }

private:
    UploadSlot* AcquireSlot(size_t bytes) {
        for (auto& s : m_pool) {
            if (!s.inUse && s.capacity >= bytes) {
                s.inUse = true;
                return &s;
            }
        }
        return nullptr; // All slots busy
    }

    ID3D12Device*            m_device = nullptr;
    std::vector<UploadSlot>  m_pool;
    std::mutex               m_mtx;
    ZeroCopyUploadStats      m_stats;
};

}} // namespace ExplorerLens::Engine
