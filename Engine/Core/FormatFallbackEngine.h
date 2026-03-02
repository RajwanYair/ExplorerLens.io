#pragma once
/**
 * @file FormatFallbackEngine.h
 * @brief Cascading format decoder selection with fallback chains.
 *
 * Purpose:
 *   When a primary decoder fails for a given file format, this engine automatically
 *   tries the next decoder in a prioritized chain. Each file extension maps to an
 *   ordered list of DecoderEntry records. The engine tracks success/failure statistics
 *   per decoder and auto-demotes decoders that exceed a configurable failure threshold.
 *
 * Classes:
 *   - FormatFallbackEngine: Main engine managing decoder chains, magic byte validation,
 *     success tracking, and automatic priority reordering.
 *
 * Key types:
 *   - DecoderEntry: Identifies a decoder with id, name, priority, and enabled flag.
 *   - FallbackStats: Aggregate statistics per extension (attempts, first-try rate, etc).
 *   - MagicSignature: Internal magic-byte pattern for header verification.
 *
 * Thread safety:
 *   All public methods are thread-safe, protected by SRWLOCK.
 *
 * Inputs:
 *   - File extension (std::wstring), decoder entries, file header bytes for magic check.
 * Outputs:
 *   - Selected decoder ID (uint32_t), statistics, ordered chains.
 *
 * Dependencies: Windows API + C++ standard library only.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Represents a single decoder that can handle a file format.
struct DecoderEntry {
    uint32_t decoderId = 0;
    std::string name;
    int32_t priority = 0;   ///< Lower value = higher priority.
    bool enabled = true;
};

/// Per-extension aggregate statistics.
struct FallbackStats {
    uint64_t totalAttempts = 0;
    uint64_t firstTrySuccesses = 0;
    uint64_t fallbackSuccesses = 0;
    uint64_t totalFailures = 0;
    double avgDecodeTimeMs = 0.0;
    double firstTrySuccessRate = 0.0;
    double fallbackRate = 0.0;
};

/// Internal: a magic-byte signature used for header verification.
struct MagicSignature {
    std::vector<uint8_t> bytes;
    size_t offset = 0;           ///< Byte offset within header to start comparing.
};

class FormatFallbackEngine {
public:
    inline FormatFallbackEngine() noexcept {
        InitializeSRWLock(&m_lock);
        m_failureThreshold = 5;
        InitBuiltinMagicDatabase();
    }

    inline ~FormatFallbackEngine() noexcept = default;

    // Non-copyable, non-movable (SRWLOCK cannot be moved).
    FormatFallbackEngine(const FormatFallbackEngine&) = delete;
    FormatFallbackEngine& operator=(const FormatFallbackEngine&) = delete;

    /// Register a decoder for the given file extension (e.g., L".jpg").
    inline void RegisterDecoder(const std::wstring& extension, DecoderEntry entry) {
        AcquireSRWLockExclusive(&m_lock);
        auto extLower = ToLower(extension);
        auto& chain = m_chains[extLower];
        // Remove existing decoder with same ID.
        chain.erase(
            std::remove_if(chain.begin(), chain.end(),
                [&](const DecoderEntry& e) { return e.decoderId == entry.decoderId; }),
            chain.end());
        chain.push_back(std::move(entry));
        SortChain(chain);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Set an explicit fallback order for an extension by decoder IDs.
    inline void SetFallbackChain(const std::wstring& extension, std::vector<uint32_t> decoderIds) {
        AcquireSRWLockExclusive(&m_lock);
        auto extLower = ToLower(extension);
        auto& chain = m_chains[extLower];
        // Reorder chain to match requested order; unmentioned decoders go to the end.
        std::vector<DecoderEntry> reordered;
        reordered.reserve(chain.size());
        int32_t pri = 0;
        for (auto id : decoderIds) {
            for (auto& e : chain) {
                if (e.decoderId == id) {
                    DecoderEntry copy = e;
                    copy.priority = pri++;
                    reordered.push_back(std::move(copy));
                    break;
                }
            }
        }
        // Append any decoders not in the explicit list.
        for (auto& e : chain) {
            bool found = false;
            for (auto& r : reordered) {
                if (r.decoderId == e.decoderId) { found = true; break; }
            }
            if (!found) {
                DecoderEntry copy = e;
                copy.priority = pri++;
                reordered.push_back(std::move(copy));
            }
        }
        chain = std::move(reordered);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Select the best decoder for an extension, optionally verifying magic bytes.
    /// Returns 0 if no suitable decoder found.
    inline uint32_t SelectDecoder(const std::wstring& extension,
        const uint8_t* headerBytes, size_t headerSize) {
        AcquireSRWLockShared(&m_lock);
        auto extLower = ToLower(extension);
        auto chainIt = m_chains.find(extLower);
        if (chainIt == m_chains.end() || chainIt->second.empty()) {
            ReleaseSRWLockShared(&m_lock);
            return 0;
        }
        // Copy chain under read lock.
        auto chain = chainIt->second;
        ReleaseSRWLockShared(&m_lock);

        // If we have header bytes and magic signatures, filter to decoders whose
        // format actually matches, then return the highest-priority one.
        bool hasMagicMatch = false;
        uint32_t magicDecoder = 0;

        for (auto& entry : chain) {
            if (!entry.enabled) continue;
            // Check if magic bytes match any signature for this extension.
            if (headerBytes != nullptr && headerSize > 0) {
                if (MatchesMagic(extLower, headerBytes, headerSize)) {
                    hasMagicMatch = true;
                    magicDecoder = entry.decoderId;
                    break;
                }
                // Also try by decoder name (lowercase).
                if (MatchesMagicByName(entry.name, headerBytes, headerSize)) {
                    hasMagicMatch = true;
                    magicDecoder = entry.decoderId;
                    break;
                }
            }
        }

        if (hasMagicMatch) return magicDecoder;

        // No magic match or no header — just return the first enabled decoder.
        for (auto& entry : chain) {
            if (entry.enabled) return entry.decoderId;
        }
        return 0;
    }

    /// Record the result of a decode attempt (for auto-reordering).
    inline void RecordDecoderResult(const std::wstring& ext, uint32_t decoderId,
        bool success, uint32_t timeMs) {
        AcquireSRWLockExclusive(&m_lock);
        auto extLower = ToLower(ext);
        auto& stats = m_decoderStats[extLower][decoderId];
        stats.attempts++;
        stats.totalTimeMs += timeMs;
        if (success) {
            stats.successes++;
            stats.consecutiveFailures = 0;
        }
        else {
            stats.failures++;
            stats.consecutiveFailures++;
            // Auto-demote if threshold exceeded.
            if (stats.consecutiveFailures >= m_failureThreshold) {
                DemoteDecoder(extLower, decoderId);
                stats.consecutiveFailures = 0;
            }
        }
        // Track extension-level stats.
        auto& extStats = m_extensionStats[extLower];
        extStats.totalAttempts++;
        if (success) {
            auto& chain = m_chains[extLower];
            bool isFirstInChain = (!chain.empty() && chain.front().decoderId == decoderId);
            if (isFirstInChain) {
                extStats.firstTrySuccesses++;
            }
            else {
                extStats.fallbackSuccesses++;
            }
        }
        else {
            extStats.totalFailures++;
        }
        extStats.totalDecodeTimeMs += timeMs;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Get aggregate statistics for all extensions.
    inline FallbackStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        FallbackStats result{};
        for (auto& [ext, es] : m_extensionStats) {
            result.totalAttempts += es.totalAttempts;
            result.firstTrySuccesses += es.firstTrySuccesses;
            result.fallbackSuccesses += es.fallbackSuccesses;
            result.totalFailures += es.totalFailures;
            result.avgDecodeTimeMs += static_cast<double>(es.totalDecodeTimeMs);
        }
        if (result.totalAttempts > 0) {
            result.avgDecodeTimeMs /= static_cast<double>(result.totalAttempts);
            result.firstTrySuccessRate =
                static_cast<double>(result.firstTrySuccesses) / static_cast<double>(result.totalAttempts);
            result.fallbackRate =
                static_cast<double>(result.fallbackSuccesses) / static_cast<double>(result.totalAttempts);
        }
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return result;
    }

    /// Get the current decoder chain for an extension.
    inline std::vector<DecoderEntry> GetChain(const std::wstring& extension) const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto extLower = ToLower(extension);
        auto it = m_chains.find(extLower);
        std::vector<DecoderEntry> result;
        if (it != m_chains.end()) result = it->second;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return result;
    }

    /// Set the failure threshold before auto-demotion.
    inline void SetFailureThreshold(uint32_t threshold) {
        AcquireSRWLockExclusive(&m_lock);
        m_failureThreshold = (std::max)(threshold, 1u);
        ReleaseSRWLockExclusive(&m_lock);
    }

private:
    struct PerDecoderStats {
        uint64_t attempts = 0;
        uint64_t successes = 0;
        uint64_t failures = 0;
        uint32_t consecutiveFailures = 0;
        uint64_t totalTimeMs = 0;
    };

    struct ExtensionStats {
        uint64_t totalAttempts = 0;
        uint64_t firstTrySuccesses = 0;
        uint64_t fallbackSuccesses = 0;
        uint64_t totalFailures = 0;
        uint64_t totalDecodeTimeMs = 0;
    };

    mutable SRWLOCK m_lock{};
    uint32_t m_failureThreshold = 5;
    std::unordered_map<std::wstring, std::vector<DecoderEntry>> m_chains;
    std::unordered_map<std::wstring, std::unordered_map<uint32_t, PerDecoderStats>> m_decoderStats;
    std::unordered_map<std::wstring, ExtensionStats> m_extensionStats;

    // Magic byte database: maps a key (extension or format name) to signatures.
    std::unordered_map<std::wstring, std::vector<MagicSignature>> m_magicDb;
    std::unordered_map<std::string, std::vector<MagicSignature>> m_magicByName;

    inline static std::wstring ToLower(const std::wstring& s) {
        std::wstring out = s;
        for (auto& c : out) {
            if (c >= L'A' && c <= L'Z') c += 32;
        }
        return out;
    }

    inline static std::string ToLowerA(const std::string& s) {
        std::string out = s;
        for (auto& c : out) {
            if (c >= 'A' && c <= 'Z') c += 32;
        }
        return out;
    }

    inline void SortChain(std::vector<DecoderEntry>& chain) {
        std::sort(chain.begin(), chain.end(),
            [](const DecoderEntry& a, const DecoderEntry& b) {
                return a.priority < b.priority;
            });
    }

    inline void DemoteDecoder(const std::wstring& extLower, uint32_t decoderId) {
        auto it = m_chains.find(extLower);
        if (it == m_chains.end()) return;
        auto& chain = it->second;
        for (size_t i = 0; i < chain.size(); ++i) {
            if (chain[i].decoderId == decoderId && i + 1 < chain.size()) {
                // Swap with next decoder.
                int32_t tmpPri = chain[i].priority;
                chain[i].priority = chain[i + 1].priority;
                chain[i + 1].priority = tmpPri;
                std::swap(chain[i], chain[i + 1]);
                break;
            }
        }
    }

    inline bool MatchesMagic(const std::wstring& extLower,
        const uint8_t* data, size_t size) const {
        auto it = m_magicDb.find(extLower);
        if (it == m_magicDb.end()) return false;
        for (auto& sig : it->second) {
            if (sig.offset + sig.bytes.size() <= size) {
                if (std::memcmp(data + sig.offset, sig.bytes.data(), sig.bytes.size()) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    inline bool MatchesMagicByName(const std::string& decoderName,
        const uint8_t* data, size_t size) const {
        auto nameL = ToLowerA(decoderName);
        auto it = m_magicByName.find(nameL);
        if (it == m_magicByName.end()) return false;
        for (auto& sig : it->second) {
            if (sig.offset + sig.bytes.size() <= size) {
                if (std::memcmp(data + sig.offset, sig.bytes.data(), sig.bytes.size()) == 0) {
                    return true;
                }
            }
        }
        return false;
    }

    inline void AddMagic(const std::wstring& ext, std::vector<uint8_t> bytes,
        size_t offset = 0) {
        m_magicDb[ext].push_back(MagicSignature{ std::move(bytes), offset });
    }

    inline void AddMagicByName(const std::string& name, std::vector<uint8_t> bytes,
        size_t offset = 0) {
        m_magicByName[ToLowerA(name)].push_back(MagicSignature{ std::move(bytes), offset });
    }

    inline void InitBuiltinMagicDatabase() {
        // JPEG: FF D8 FF
        AddMagic(L".jpg", { 0xFF, 0xD8, 0xFF });
        AddMagic(L".jpeg", { 0xFF, 0xD8, 0xFF });
        AddMagicByName("jpeg", { 0xFF, 0xD8, 0xFF });

        // PNG: 89 50 4E 47
        AddMagic(L".png", { 0x89, 0x50, 0x4E, 0x47 });
        AddMagicByName("png", { 0x89, 0x50, 0x4E, 0x47 });

        // GIF: 47 49 46 38
        AddMagic(L".gif", { 0x47, 0x49, 0x46, 0x38 });
        AddMagicByName("gif", { 0x47, 0x49, 0x46, 0x38 });

        // BMP: 42 4D
        AddMagic(L".bmp", { 0x42, 0x4D });
        AddMagicByName("bmp", { 0x42, 0x4D });

        // WebP: RIFF....WEBP  (RIFF at 0, WEBP at 8)
        AddMagic(L".webp", { 0x52, 0x49, 0x46, 0x46 });
        AddMagicByName("webp", { 0x57, 0x45, 0x42, 0x50 }, 8);

        // TIFF LE: 49 49 2A 00
        AddMagic(L".tiff", { 0x49, 0x49, 0x2A, 0x00 });
        AddMagic(L".tif", { 0x49, 0x49, 0x2A, 0x00 });
        // TIFF BE: 4D 4D 00 2A
        AddMagic(L".tiff", { 0x4D, 0x4D, 0x00, 0x2A });
        AddMagic(L".tif", { 0x4D, 0x4D, 0x00, 0x2A });
        AddMagicByName("tiff", { 0x49, 0x49, 0x2A, 0x00 });
        AddMagicByName("tiff", { 0x4D, 0x4D, 0x00, 0x2A });

        // PSD: 38 42 50 53
        AddMagic(L".psd", { 0x38, 0x42, 0x50, 0x53 });
        AddMagicByName("psd", { 0x38, 0x42, 0x50, 0x53 });

        // PDF: %PDF (25 50 44 46)
        AddMagic(L".pdf", { 0x25, 0x50, 0x44, 0x46 });
        AddMagicByName("pdf", { 0x25, 0x50, 0x44, 0x46 });

        // ZIP: 50 4B 03 04
        AddMagic(L".zip", { 0x50, 0x4B, 0x03, 0x04 });
        AddMagicByName("zip", { 0x50, 0x4B, 0x03, 0x04 });

        // RAR: 52 61 72 21 1A 07
        AddMagic(L".rar", { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07 });
        AddMagicByName("rar", { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07 });

        // 7z: 37 7A BC AF 27 1C
        AddMagic(L".7z", { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C });
        AddMagicByName("7z", { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C });

        // AVIF: ftyp box at offset 4 with brand "avif" or "mif1"
        AddMagic(L".avif", { 0x66, 0x74, 0x79, 0x70, 0x61, 0x76, 0x69, 0x66 }, 4); // ftypavif
        AddMagic(L".avif", { 0x66, 0x74, 0x79, 0x70, 0x6D, 0x69, 0x66, 0x31 }, 4); // ftypmif1
        AddMagicByName("avif", { 0x66, 0x74, 0x79, 0x70, 0x61, 0x76, 0x69, 0x66 }, 4);

        // HEIC: ftyp box at offset 4 with brand "heic" or "heix"
        AddMagic(L".heic", { 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 }, 4); // ftypheic
        AddMagic(L".heic", { 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x78 }, 4); // ftypheix
        AddMagicByName("heic", { 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 }, 4);

        // JPEG XL: FF 0A (codestream) or container 00 00 00 0C 4A 58 4C 20
        AddMagic(L".jxl", { 0xFF, 0x0A });
        AddMagic(L".jxl", { 0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20 });
        AddMagicByName("jxl", { 0xFF, 0x0A });
        AddMagicByName("jxl", { 0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20 });
    }
};

} // namespace Engine
} // namespace ExplorerLens
