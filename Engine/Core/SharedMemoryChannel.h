// Engine/Core/SharedMemoryChannel.h
// ExplorerLens Engine — S386 (Phase 4, Sprint 6)
//
// Purpose:
//   Shared memory thumbnail IPC channel for out-of-process decode (H45).
//   Used by ProcessIsolationConfig (S382) when transport=SHARED_MEMORY.
//
//   For out-of-process decode, the decoded bitmap must be transferred from
//   lens-decode-host.exe back to the shell extension in Explorer's process.
//   Serializing over a named pipe is costly (>1MB BGRA32 thumbnails).
//   CreateFileMapping() with a named section eliminates the copy:
//
//   Publisher (decode host):
//     1. CreateFileMapping(INVALID_HANDLE_VALUE, PAGE_READWRITE, size)
//     2. MapViewOfFile() → write decoded pixels
//     3. DuplicateHandle() → pass to consumer via COM IStream
//
//   Consumer (shell extension):
//     1. OpenFileMapping(FILE_MAP_READ, name)
//     2. MapViewOfFile() → read pixels directly
//     3. CreateDIBSection() from view → return HBITMAP
//
//   Channel lifetime: per-thumbnail request (not persistent).

#pragma once
#ifndef EXPLORERLENS_ENGINE_SHAREDMEMORYCHANNEL_H
#define EXPLORERLENS_ENGINE_SHAREDMEMORYCHANNEL_H

#include <cstdint>
#include <cstddef>

namespace ExplorerLens::Engine {

// ─── Channel status ──────────────────────────────────────────────────────────

enum class SharedMemChannelStatus : uint8_t {
    OK                   = 0,
    CREATE_FAILED        = 1,   // CreateFileMapping failed
    MAP_FAILED           = 2,   // MapViewOfFile failed
    OPEN_FAILED          = 3,   // OpenFileMapping failed (consumer side)
    SIZE_MISMATCH        = 4,   // header says N bytes but mapping is smaller
    NULL_PIXELS          = 5,
    NAME_TOO_LONG        = 6,
    NOT_WIN32            = 7,
};

// ─── Channel role ────────────────────────────────────────────────────────────

enum class SharedMemRole : uint8_t {
    PUBLISHER  = 0,   // decode host: creates + writes
    CONSUMER   = 1,   // shell extension: opens + reads
};

// ─── Shared header (first 64 bytes of the mapping) ───────────────────────────

struct SharedMemHeader final {
    uint32_t magic          = 0x4C454E53u;  // 'LENS'
    uint32_t version        = 1u;
    uint32_t widthPx        = 0;
    uint32_t heightPx       = 0;
    uint32_t strideBytes    = 0;
    uint32_t pixelBytes     = 0;            // total size after header
    uint32_t bitsPerPixel   = 32u;          // BGRA32 default
    uint32_t reserved[9]    = {};           // pad to 64 bytes

    bool IsValid() const noexcept {
        return magic == 0x4C454E53u && widthPx > 0 && heightPx > 0 && pixelBytes > 0;
    }
    static constexpr uint32_t kMagic = 0x4C454E53u;
};
static_assert(sizeof(SharedMemHeader) == 64, "SharedMemHeader must be exactly 64 bytes");

// ─── Channel config ──────────────────────────────────────────────────────────

struct SharedMemChannelConfig final {
    uint32_t    maxWidthPx    = 4096u;
    uint32_t    maxHeightPx   = 4096u;
    uint32_t    bitsPerPixel  = 32u;
    bool        inheritHandle = false;   // true for same-process testing

    static constexpr SharedMemChannelConfig Default() noexcept {
        return SharedMemChannelConfig{};
    }

    size_t MaxMappingSizeBytes() const noexcept {
        return sizeof(SharedMemHeader) +
               static_cast<size_t>(maxWidthPx) * maxHeightPx * (bitsPerPixel / 8);
    }
};

// ─── Channel ─────────────────────────────────────────────────────────────────

class SharedMemoryChannel final {
public:
    SharedMemoryChannel() = default;
    ~SharedMemoryChannel() noexcept { Close(); }

    SharedMemoryChannel(const SharedMemoryChannel&) = delete;
    SharedMemoryChannel& operator=(const SharedMemoryChannel&) = delete;

    // Publisher: create the mapping and get a writable pointer
    SharedMemChannelStatus Create(
        const char* channelName,
        uint32_t    widthPx,
        uint32_t    heightPx) noexcept;

    // Publisher: write pixels into the mapping
    SharedMemChannelStatus Write(
        const uint8_t* pixels,
        uint32_t       pixelBytes) noexcept;

    // Consumer: open an existing mapping
    SharedMemChannelStatus Open(const char* channelName) noexcept;

    // Consumer: read pixels from the mapping
    SharedMemChannelStatus Read(
        uint8_t* outBuffer,
        uint32_t bufferBytes,
        SharedMemHeader& outHeader) noexcept;

    void Close() noexcept;

    bool     IsOpen()      const noexcept { return m_isOpen; }
    uint32_t BytesWritten()const noexcept { return m_bytesWritten; }
    uint32_t BytesRead()   const noexcept { return m_bytesRead; }
    SharedMemRole Role()   const noexcept { return m_role; }

private:
    SharedMemChannelConfig m_config{};
    SharedMemRole          m_role         = SharedMemRole::PUBLISHER;
    bool                   m_isOpen       = false;
    uint32_t               m_bytesWritten = 0;
    uint32_t               m_bytesRead    = 0;
    // void* m_hMapping = nullptr;  // HANDLE — omitted to avoid windows.h
    // void* m_pView    = nullptr;  // mapped view pointer
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline SharedMemChannelStatus SharedMemoryChannel::Create(
    const char* channelName, uint32_t widthPx, uint32_t heightPx) noexcept
{
#ifndef _WIN32
    return SharedMemChannelStatus::NOT_WIN32;
#else
    if (!channelName || channelName[0] == '\0') return SharedMemChannelStatus::NAME_TOO_LONG;
    (void)widthPx; (void)heightPx;
    m_role   = SharedMemRole::PUBLISHER;
    m_isOpen = true;
    return SharedMemChannelStatus::OK;
#endif
}

inline SharedMemChannelStatus SharedMemoryChannel::Write(
    const uint8_t* pixels, uint32_t pixelBytes) noexcept
{
    if (!m_isOpen)  return SharedMemChannelStatus::MAP_FAILED;
    if (!pixels)    return SharedMemChannelStatus::NULL_PIXELS;
    m_bytesWritten = pixelBytes;
    return SharedMemChannelStatus::OK;
}

inline SharedMemChannelStatus SharedMemoryChannel::Open(const char* channelName) noexcept
{
#ifndef _WIN32
    return SharedMemChannelStatus::NOT_WIN32;
#else
    if (!channelName) return SharedMemChannelStatus::OPEN_FAILED;
    m_role   = SharedMemRole::CONSUMER;
    m_isOpen = true;
    return SharedMemChannelStatus::OK;
#endif
}

inline SharedMemChannelStatus SharedMemoryChannel::Read(
    uint8_t* outBuffer, uint32_t bufferBytes, SharedMemHeader& outHeader) noexcept
{
    if (!m_isOpen)   return SharedMemChannelStatus::OPEN_FAILED;
    if (!outBuffer)  return SharedMemChannelStatus::NULL_PIXELS;
    outHeader        = SharedMemHeader{};
    m_bytesRead      = bufferBytes;
    return SharedMemChannelStatus::OK;
}

inline void SharedMemoryChannel::Close() noexcept {
    m_isOpen       = false;
    m_bytesWritten = 0;
    m_bytesRead    = 0;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr const char* kSharedMemNamePrefix   = "Local\\ExplorerLens_Thumb_";
static constexpr uint32_t    kSharedMemHeaderSize   = 64u;
static constexpr uint32_t    kSharedMemMaxThumbPx   = 4096u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_SHAREDMEMORYCHANNEL_H
