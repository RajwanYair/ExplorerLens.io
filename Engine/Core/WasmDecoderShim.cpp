// WasmDecoderShim.cpp — WebAssembly Decoder Entry-Point Shim
// Copyright (c) 2026 ExplorerLens Project
//
#include "WasmDecoderShim.h"

#include <algorithm>
#include <stdexcept>

namespace ExplorerLens { namespace Engine {

uint32_t WasmDecoderShim::RegisterDecoder(const WasmDecoderConfig& config)
{
    const uint32_t handle = m_nextHandle++;
    m_decoders.push_back({ handle, config });
    return handle;
}

void WasmDecoderShim::UnregisterDecoder(uint32_t handle)
{
    m_decoders.erase(
        std::remove_if(m_decoders.begin(), m_decoders.end(),
            [handle](const Entry& e) { return e.handle == handle; }),
        m_decoders.end());
}

WasmDecodeResult WasmDecoderShim::Decode(uint32_t       handle,
                                          const uint8_t* /*data*/,
                                          size_t         /*dataSize*/) const
{
    const auto it = std::find_if(m_decoders.begin(), m_decoders.end(),
        [handle](const Entry& e) { return e.handle == handle; });

    if (it == m_decoders.end())
    {
        WasmDecodeResult err;
        err.success  = false;
        err.errorMsg = "Unknown decoder handle";
        return err;
    }

    const uint32_t w = (it->config.maxWidthPx  > 0) ? it->config.maxWidthPx  : 32u;
    const uint32_t h = (it->config.maxHeightPx > 0) ? it->config.maxHeightPx : 32u;

    WasmDecodeResult result;
    result.width  = w;
    result.height = h;
    result.pixelBuffer.assign(static_cast<size_t>(w) * h * 4u, 0xFFu);
    result.success = true;
    return result;
}

uint32_t WasmDecoderShim::DecoderCount() const
{
    return static_cast<uint32_t>(m_decoders.size());
}

}} // namespace ExplorerLens::Engine
