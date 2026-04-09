// ZeroCopyGPUSurface.h — Zero-Copy CPU→GPU BGRA Surface
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a write-combined CPU-visible, GPU-readable surface for handing
// decoded BGRA32 thumbnail pixels to the Windows Shell (IThumbnailProvider)
// without an extra CPU-side copy. Uses D3D11_MAP_WRITE_NO_OVERWRITE /
// D3D12_HEAP_TYPE_UPLOAD semantics for zero-copy transfer.
//
#pragma once
#include <cstdint>
#include <memory>

namespace ExplorerLens { namespace Engine {

enum class SurfaceAllocMode : uint8_t {
    Default       = 0, // Automatically choose best mode
    D3D11Upload   = 1, // D3D11 STAGING → read-back to CPU WC memory
    D3D12Upload   = 2, // D3D12 UPLOAD heap (write-combined)
    SystemMemory  = 3, // Plain malloc (fallback if no GPU)
};

struct SurfaceDesc {
    uint32_t       width       = 0;
    uint32_t       height      = 0;
    uint32_t       rowPitch    = 0;  // Bytes per row (may be padded)
    uint8_t*       pData       = nullptr; // Mapped pointer (valid between Map/Unmap)
    SurfaceAllocMode allocMode = SurfaceAllocMode::Default;
};

class ZeroCopyGPUSurface {
public:
    ZeroCopyGPUSurface();
    ~ZeroCopyGPUSurface();

    // Allocate a surface for the given dimensions (BGRA32 = 4 bytes/px).
    // Returns false if allocation fails.
    bool Allocate(uint32_t width, uint32_t height,
                  SurfaceAllocMode mode = SurfaceAllocMode::Default) noexcept;

    // Map surface for CPU write. Returns non-null pointer on success.
    uint8_t* Map() noexcept;

    // Unmap after CPU writes are complete; flushes write-combine buffer.
    void Unmap() noexcept;

    // Copy from a CPU BGRA buffer into the mapped surface in one call.
    // width/height must match the allocated dimensions.
    bool CopyFrom(const uint8_t* srcBGRA, uint32_t srcRowPitch) noexcept;

    const SurfaceDesc& GetDesc()  const noexcept { return m_desc; }
    bool               IsValid()  const noexcept { return m_desc.pData != nullptr && m_mapped; }
    SurfaceAllocMode   AllocMode() const noexcept { return m_desc.allocMode; }

    // Release backing memory / GPU resource.
    void Release() noexcept;

private:
    bool AllocD3D11() noexcept;
    bool AllocD3D12() noexcept;
    bool AllocSystem() noexcept;

    SurfaceDesc            m_desc{};
    bool                   m_mapped = false;
    struct Impl;
    std::unique_ptr<Impl>  m_impl;
};

}} // namespace ExplorerLens::Engine
