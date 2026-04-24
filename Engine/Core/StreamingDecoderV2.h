//==============================================================================
// ExplorerLens Engine — Streaming decoder v2 contract (Sprint S245)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A-series / ADR-015 — IStreamingDecoder for files >50 MB.
//==============================================================================
//
// Why v2:
//   The existing IStreamingDecoder.h (Engine/Core/) defines the basic
//   ProbeResult/ProbeHeader/DecodeAtSize shape. v2 adds the chunked-pull
//   protocol needed for PSD/TIF files in the 100 MB — 4 GB range, plus a
//   cancellation hook (S242). It does NOT replace v1 — v2 is a sibling
//   interface that v1 decoders MAY opt into.
//
// Protocol (simplified):
//     OpenStream(handle) -> StreamHandle
//     RequestChunk(h, offset, len, cancelToken) -> ChunkResult
//     DecodeAtSize(h, thumbPx, cancelToken) -> DecodedThumb
//     CloseStream(h)
//
// Header-only contract. No implementations yet. All types are POD + trivially
// copyable so they can cross the COM boundary.
//==============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "DecodeCancelToken.h"

namespace ExplorerLens {
namespace Engine {

/// <summary>Opaque handle returned by OpenStream.</summary>
struct StreamingDecoderHandle
{
    std::uint64_t opaque = 0;
    constexpr bool IsValid() const noexcept { return opaque != 0; }
};

/// <summary>Result of a chunk pull.</summary>
enum class StreamingChunkStatus : std::uint8_t
{
    OK          = 0,
    EOF_REACHED = 1,
    IO_ERROR    = 2,
    CANCELLED   = 3,
    TIMEOUT     = 4,
    OUT_OF_BUDGET = 5
};

/// <summary>Bytes-pulled record — populated by RequestChunk.</summary>
struct StreamingChunkResult
{
    StreamingChunkStatus status = StreamingChunkStatus::IO_ERROR;
    std::uint64_t        offset = 0;
    std::uint32_t        bytesReturned = 0;
    std::uint32_t        bytesRequested = 0;
};

/// <summary>Advisory progress callback — wired to Explorer's progress UI.</summary>
struct StreamingV2Progress
{
    std::uint64_t bytesConsumed = 0;
    std::uint64_t bytesTotal    = 0;
    std::uint32_t currentPhase  = 0;   // 0=probe 1=hdr 2=body 3=finalise
    std::uint32_t percent       = 0;   // 0..100
};

/// <summary>Minimum file size that routes through the streaming path.</summary>
inline constexpr std::uint64_t kStreamingThresholdBytes = 50ull * 1024ull * 1024ull;

/// <summary>Default chunk size. 1 MiB balances I/O throughput vs cancel latency.</summary>
inline constexpr std::uint32_t kStreamingDefaultChunkBytes = 1u << 20;

/// <summary>Max in-flight chunks per stream. Back-pressure knob.</summary>
inline constexpr std::uint32_t kStreamingMaxInflight = 4;

static_assert(std::is_trivially_copyable_v<StreamingDecoderHandle>,
              "StreamingDecoderHandle must be trivially copyable");
static_assert(std::is_trivially_copyable_v<StreamingChunkResult>,
              "StreamingChunkResult must be trivially copyable");
static_assert(std::is_trivially_copyable_v<StreamingV2Progress>,
              "StreamingV2Progress must be trivially copyable");

} // namespace Engine
} // namespace ExplorerLens
