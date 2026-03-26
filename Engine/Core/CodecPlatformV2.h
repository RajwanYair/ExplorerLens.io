// CodecPlatformV2.h — Pluggable Codec Registry with Capability Negotiation
// Copyright (c) 2026 ExplorerLens Project
//
// Central codec registry for ExplorerLens v18+. Replaces per-format if/else
// dispatch with a capability-negotiation model: decoders advertise their
// supported extensions, required features, and hardware constraints.
//
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CodecCapability : uint32_t {
    None            = 0,
    HardwareGPU     = 1 << 0,
    HDRSupport      = 1 << 1,
    AnimationFrames = 1 << 2,
    Lossless        = 1 << 3,
    AlphaChannel    = 1 << 4,
    MultiPage       = 1 << 5,
    SignedInt       = 1 << 6,
    FloatPixels     = 1 << 7,
    BidirectionalSeek = 1 << 8,
    Streaming       = 1 << 9,
};

inline CodecCapability operator|(CodecCapability a, CodecCapability b) {
    return static_cast<CodecCapability>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct CodecDescriptor {
    std::string         id;           // unique codec id, e.g. "avif_dav1d"
    std::string         displayName;
    std::vector<std::string> extensions;   // {".avif", ".avis"}
    std::vector<std::string> mimeTypes;
    CodecCapability     capabilities{CodecCapability::None};
    uint32_t            priority{100};     // lower = preferred when multiple codecs handle same ext
    bool                isBuiltIn{true};
    bool                requiresGPU{false};
    bool                threadSafe{true};
};

struct CodecDecodeRequest {
    const void* data{nullptr};
    size_t      size{0};
    uint32_t    reqWidth{256};
    uint32_t    reqHeight{256};
    uint32_t    frameIndex{0};
    uint32_t    flags{0};
};

struct CodecDecodeResult {
    std::vector<uint8_t> bgra;
    uint32_t             width{0};
    uint32_t             height{0};
    uint32_t             stride{0};
    std::string          codecId;
    double               decodeMs{0.0};
    bool                 fromGPU{false};
};

using CodecDecodeFn = std::function<std::optional<CodecDecodeResult>(const CodecDecodeRequest&)>;
using CodecProbeFn  = std::function<bool(std::span<const uint8_t>)>;

struct CodecRegistration {
    CodecDescriptor  descriptor;
    CodecDecodeFn    decode;
    CodecProbeFn     probe;       // magic-byte probe (fast, no alloc)
};

class CodecPlatformV2 {
public:
    static CodecPlatformV2& Instance() noexcept {
        static CodecPlatformV2 s_inst;
        return s_inst;
    }

    // Register a codec — used by built-in decoders and plugins.
    void Register(CodecRegistration reg);

    // Unregister by codec id — used when a plugin is unloaded.
    void Unregister(const std::string& codecId);

    // Find the best codec for a given extension.
    [[nodiscard]] const CodecRegistration* FindForExtension(
        const std::string& ext) const noexcept;

    // Auto-probe by magic bytes (ignores file extension).
    [[nodiscard]] const CodecRegistration* ProbeByMagic(
        std::span<const uint8_t> header) const noexcept;

    // Decode using best codec (extension first, then magic probe fallback).
    [[nodiscard]] std::optional<CodecDecodeResult> Decode(
        const std::string& ext,
        const CodecDecodeRequest& req) const;

    [[nodiscard]] std::vector<CodecDescriptor> ListAll() const;
    [[nodiscard]] size_t                       Count() const noexcept;

    // Called at startup to register all built-in codecs.
    void RegisterBuiltIns();

private:
    CodecPlatformV2() = default;

    std::vector<CodecRegistration> m_codecs;
    mutable std::mutex             m_mutex;
};

} // namespace Engine
} // namespace ExplorerLens
