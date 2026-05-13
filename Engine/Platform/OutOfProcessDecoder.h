// =============================================================================
// ExplorerLens Engine — OutOfProcessDecoder.h
// Sprint S357 | ROADMAP v8.0 Phase 4 (Process Isolation — H36)
// Out-of-process decoder proxy for unstable or high-risk format decoders.
//
// Phase 4 exit criterion: "Process isolation option for decoders (H36)"
// Routes decode requests to a helper process (LensDecodeHost.exe) via
// anonymous pipe IPC. If the child crashes, the parent is unaffected.
//
// Architecture:
//   COM server (LENSShell.dll) → OutOfProcessDecoder (pipe IPC)
//                               → LensDecodeHost.exe → actual decoder
//
// The child process produces a BGRA pixel buffer + width/height, writes it
// to a shared-memory segment, then signals the parent via pipe.
//
// Windows-only. Non-Windows builds compile to stubs.
// =============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#ifndef EXPLORERLENS_ENGINE_OUTOFPROCESSDECODER_H
#define EXPLORERLENS_ENGINE_OUTOFPROCESSDECODER_H

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// OutOfProcDecodeStatus — outcome codes for remote decode requests
// ---------------------------------------------------------------------------
enum class OutOfProcDecodeStatus : uint8_t {
    OK                  = 0,  ///< Decoded successfully
    NOT_WIN32           = 1,  ///< Non-Windows — OOP decode not available
    SPAWN_FAIL          = 2,  ///< CreateProcess for LensDecodeHost.exe failed
    PIPE_CREATE_FAIL    = 3,  ///< Could not create anonymous pipes
    PIPE_WRITE_FAIL     = 4,  ///< Failed to write decode request to pipe
    TIMEOUT             = 5,  ///< Child did not respond within deadline
    CHILD_CRASHED       = 6,  ///< Child exited with non-zero code
    SHMEM_FAIL          = 7,  ///< Could not create/map shared memory segment
    DECODE_FAILED       = 8,  ///< Child reported a decode error
    HOST_NOT_FOUND      = 9,  ///< LensDecodeHost.exe binary not found
    IPC_PROTOCOL_ERROR  = 10, ///< Unexpected message format from child
    NULL_INPUT          = 11, ///< Input file path or buffer is null
};

// ---------------------------------------------------------------------------
// OutOfProcDecodeResult — pixel data returned by the child process
// ---------------------------------------------------------------------------
struct OutOfProcDecodeResult final {
    OutOfProcDecodeStatus status{OutOfProcDecodeStatus::NOT_WIN32};
    uint32_t              width{0};
    uint32_t              height{0};
    std::vector<uint8_t>  pixels;     ///< BGRA8 pixel data (width * height * 4 bytes)
    uint32_t              childExitCode{0};
    uint64_t              elapsedMs{0};

    [[nodiscard]] bool Success()    const noexcept { return status == OutOfProcDecodeStatus::OK; }
    [[nodiscard]] bool HasPixels()  const noexcept {
        return Success() && width > 0 && height > 0
            && pixels.size() == static_cast<size_t>(width) * height * 4u;
    }
};

// ---------------------------------------------------------------------------
// OutOfProcDecodeConfig — IPC + isolation parameters
// ---------------------------------------------------------------------------
struct OutOfProcDecodeConfig final {
    uint32_t     timeoutMs{5000u};       ///< Max wait for child response
    uint32_t     maxOutputBytes{256u * 1024u * 1024u}; ///< 256 MB shmem cap
    std::wstring hostExePath{L""};       ///< Empty = auto-locate next to current module
    bool         inheritHandles{false};  ///< CreateProcess bInheritHandles
    bool         killOnTimeout{true};    ///< TerminateProcess if timeout fires
    bool         reuseProcess{false};    ///< Keep child alive for next decode
    uint32_t     maxWidth{4096u};        ///< Thumbnail output size cap
    uint32_t     maxHeight{4096u};

    [[nodiscard]] static OutOfProcDecodeConfig Default() noexcept {
        return OutOfProcDecodeConfig{};
    }

    [[nodiscard]] static OutOfProcDecodeConfig ForUntrustedFormat() noexcept {
        OutOfProcDecodeConfig c;
        c.timeoutMs      = 3000u;
        c.killOnTimeout  = true;
        c.reuseProcess   = false;
        c.maxOutputBytes = 64u * 1024u * 1024u; // 64 MB cap for untrusted
        c.maxWidth       = 2048u;
        c.maxHeight      = 2048u;
        return c;
    }
};

// ---------------------------------------------------------------------------
// IPC message types exchanged over the pipe (fixed-layout POD)
// ---------------------------------------------------------------------------
static constexpr uint32_t kOopDecodeRequestMagic  = 0x4C454E53u; ///< 'LENS'
static constexpr uint32_t kOopDecodeResponseMagic = 0x4C454E52u; ///< 'LENR'
static constexpr uint32_t kOopDecodeProtocolVer   = 1u;
static constexpr uint32_t kOopDecodeMaxPathBytes  = 2048u;

#pragma pack(push, 1)
struct OopDecodeRequest final {
    uint32_t magic{kOopDecodeRequestMagic};
    uint32_t protocolVersion{kOopDecodeProtocolVer};
    uint32_t thumbWidth{256u};
    uint32_t thumbHeight{256u};
    uint32_t pathLengthBytes{0};
    // Followed immediately by pathLengthBytes of UTF-16LE path data
};

struct OopDecodeResponse final {
    uint32_t magic{kOopDecodeResponseMagic};
    uint32_t protocolVersion{kOopDecodeProtocolVer};
    uint32_t statusCode{0};     ///< OutOfProcDecodeStatus value
    uint32_t pixelWidth{0};
    uint32_t pixelHeight{0};
    uint64_t shmemBytes{0};     ///< Byte size of shared-memory payload
    wchar_t  shmemName[64]{};   ///< Named shared-memory object name
};
#pragma pack(pop)

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t kOopDefaultTimeoutMs    = 5000u;
static constexpr uint32_t kOopMaxShmemBytes       = 256u * 1024u * 1024u;
static constexpr wchar_t  kOopHostExeName[]       = L"LensDecodeHost.exe";
static constexpr uint32_t kOopPipeBufferBytes     = 4096u;

// ---------------------------------------------------------------------------
// OutOfProcessDecoder — proxy that spawns LensDecodeHost.exe for isolation
// ---------------------------------------------------------------------------
class OutOfProcessDecoder final {
public:
    explicit OutOfProcessDecoder(
        OutOfProcDecodeConfig cfg = OutOfProcDecodeConfig::Default()) noexcept;
    ~OutOfProcessDecoder() noexcept;

    // Non-copyable; move is allowed
    OutOfProcessDecoder(const OutOfProcessDecoder&)            = delete;
    OutOfProcessDecoder& operator=(const OutOfProcessDecoder&) = delete;
    OutOfProcessDecoder(OutOfProcessDecoder&&)                 noexcept;
    OutOfProcessDecoder& operator=(OutOfProcessDecoder&&)      noexcept;

    /// Decode a file via the out-of-process host.
    /// @param filePath   Full path to the file to decode
    /// @param thumbW     Requested thumbnail width
    /// @param thumbH     Requested thumbnail height
    [[nodiscard]] OutOfProcDecodeResult Decode(
        const std::wstring& filePath,
        uint32_t thumbW,
        uint32_t thumbH) noexcept;

    /// Check that LensDecodeHost.exe is accessible.
    [[nodiscard]] bool IsHostAvailable() const noexcept;

    /// Returns true if a reused child process is currently alive.
    [[nodiscard]] bool HasLiveChild() const noexcept;

    /// Kill any running child process.
    void KillChild() noexcept;

private:
    OutOfProcDecodeConfig m_cfg;
    void*                 m_hProcess{nullptr}; ///< HANDLE (opaque on non-Win32)
    void*                 m_hPipeRead{nullptr};
    void*                 m_hPipeWrite{nullptr};
};

// ---------------------------------------------------------------------------
// Inline stub implementations for non-Windows
// ---------------------------------------------------------------------------
#ifndef _WIN32

inline OutOfProcessDecoder::OutOfProcessDecoder(OutOfProcDecodeConfig cfg) noexcept
    : m_cfg(cfg) {}
inline OutOfProcessDecoder::~OutOfProcessDecoder() noexcept {}
inline OutOfProcessDecoder::OutOfProcessDecoder(OutOfProcessDecoder&&) noexcept {}
inline OutOfProcessDecoder& OutOfProcessDecoder::operator=(OutOfProcessDecoder&&) noexcept { return *this; }

inline OutOfProcDecodeResult OutOfProcessDecoder::Decode(
    const std::wstring& /*filePath*/,
    uint32_t /*thumbW*/,
    uint32_t /*thumbH*/) noexcept
{
    OutOfProcDecodeResult r;
    r.status = OutOfProcDecodeStatus::NOT_WIN32;
    return r;
}

inline bool OutOfProcessDecoder::IsHostAvailable() const noexcept { return false; }
inline bool OutOfProcessDecoder::HasLiveChild()    const noexcept { return false; }
inline void OutOfProcessDecoder::KillChild()       noexcept {}

#endif // !_WIN32

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_OUTOFPROCESSDECODER_H
