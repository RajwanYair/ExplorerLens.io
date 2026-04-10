// WasmDecoderShim.h — WebAssembly Decoder Entry-Point Shim
// Copyright (c) 2026 ExplorerLens Project
//
// Thin C ABI shim that wraps engine decoder entry points in a WASM-compilable
// interface, enabling format decode stubs to run in-browser without server
// round-trips for SharePoint / OneDrive web thumbnails.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class WasmTargetPlatform : uint32_t
{
    WASM32 = 0,
    WASM64 = 1,
    NATIVE = 2
};

struct WasmDecoderConfig
{
    std::string       format;          // e.g. "jpeg", "png", "webp"
    uint32_t          maxWidthPx  = 0;
    uint32_t          maxHeightPx = 0;
    WasmTargetPlatform targetPlatform = WasmTargetPlatform::WASM32;
};

struct WasmDecodeResult
{
    uint32_t             width       = 0;
    uint32_t             height      = 0;
    std::vector<uint8_t> pixelBuffer;
    bool                 success     = false;
    std::string          errorMsg;
};

class WasmDecoderShim
{
public:
    WasmDecoderShim()  = default;
    ~WasmDecoderShim() = default;

    WasmDecoderShim(const WasmDecoderShim&)            = delete;
    WasmDecoderShim& operator=(const WasmDecoderShim&) = delete;
    WasmDecoderShim(WasmDecoderShim&&)                 = default;
    WasmDecoderShim& operator=(WasmDecoderShim&&)      = default;

    uint32_t RegisterDecoder(const WasmDecoderConfig& config);
    void     UnregisterDecoder(uint32_t handle);

    WasmDecodeResult Decode(uint32_t          handle,
                            const uint8_t*    data,
                            size_t            dataSize) const;

    uint32_t DecoderCount() const;

private:
    struct Entry { uint32_t handle; WasmDecoderConfig config; };
    std::vector<Entry> m_decoders;
    uint32_t           m_nextHandle = 1u;
};

}} // namespace ExplorerLens::Engine
