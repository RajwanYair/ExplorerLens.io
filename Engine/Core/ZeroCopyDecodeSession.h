// ZeroCopyDecodeSession.h — Zero-copy decode session context
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks the lifecycle of a single zero-copy thumbnail decode from
// DirectStorage GPU staging buffer through GPU decompression, GPU decode,
// and BGRA blit to the output surface. One session per thumbnail request;
// sessions are pooled by ZeroCopyPipeline for reuse across requests.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class ZCSessionState : uint8_t {
    IDLE, IO_PENDING, DECOMPRESS_PENDING, DECODE_PENDING, COMPLETE, FAILED
};

struct ZeroCopyDecodeSession {
    uint32_t       sessionId     = 0;
    std::wstring   filePath;
    uint32_t       thumbSize     = 256;
    ZCSessionState state         = ZCSessionState::IDLE;
    float          ioMs          = 0.0f;
    float          decompressMs  = 0.0f;
    float          decodeMs      = 0.0f;
    uint8_t*       stagingPtr    = nullptr;   // GPU staging buffer (not owned)
    uint32_t       stagingBytes  = 0;
    bool           gpuDecompress = false;

    float TotalMs()    const noexcept { return ioMs + decompressMs + decodeMs; }
    bool  IsTerminal() const noexcept {
        return state == ZCSessionState::COMPLETE || state == ZCSessionState::FAILED;
    }

    static const char* StateName(ZCSessionState s) noexcept
    {
        switch (s)
        {
            case ZCSessionState::IDLE:                return "Idle";
            case ZCSessionState::IO_PENDING:          return "IO_Pending";
            case ZCSessionState::DECOMPRESS_PENDING:  return "Decompress_Pending";
            case ZCSessionState::DECODE_PENDING:      return "Decode_Pending";
            case ZCSessionState::COMPLETE:            return "Complete";
            default:                                  return "Failed";
        }
    }
};

}} // namespace ExplorerLens::Engine
