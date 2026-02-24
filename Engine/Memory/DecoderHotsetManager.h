#pragma once
//==============================================================================
// DecoderHotsetManager.h
// Single-format hot mode runtime: load only required decoders for dominant
// format family, defer all others to cold state, timed unload for inactive.
//==============================================================================

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens { namespace Memory {

/// Decoder load state
enum class DecoderLoadState : uint8_t {
    Cold       = 0,  // Not loaded, no memory used
    Warming    = 1,  // Load in progress
    Hot        = 2,  // Fully loaded and active
    Cooling    = 3,  // Marked for unload after idle timeout
    Evicted    = 4   // Forcibly unloaded due to memory pressure
};

/// Decoder entry in the hotset registry
struct DecoderHotsetEntry {
    std::string decoderName;
    std::string libraryPath;     // DLL path for modular codecs
    DecoderLoadState state = DecoderLoadState::Cold;
    size_t memoryFootprintBytes = 0;
    std::chrono::steady_clock::time_point lastUsed;
    std::chrono::steady_clock::time_point loadedAt;
    int useCount = 0;
    std::vector<std::string> extensions;  // Extensions this decoder handles

    bool IsActive() const { return state == DecoderLoadState::Hot; }
    bool IsIdle(std::chrono::seconds threshold) const {
        if (state != DecoderLoadState::Hot) return false;
        auto elapsed = std::chrono::steady_clock::now() - lastUsed;
        return elapsed > threshold;
    }
    double IdleSeconds() const {
        auto elapsed = std::chrono::steady_clock::now() - lastUsed;
        return std::chrono::duration<double>(elapsed).count();
    }
};

/// Hotset activation mode
enum class HotsetMode : uint8_t {
    AllDecoders     = 0,  // Load all decoders eagerly (legacy behavior)
    DominantOnly    = 1,  // Load only dominant format family decoders
    TopN            = 2,  // Load top N most-used decoders
    OnDemand        = 3   // Load only when first request for format arrives
};

/// Hotset configuration
struct HotsetConfig {
    HotsetMode mode = HotsetMode::DominantOnly;
    int topN = 3;
    std::chrono::seconds idleTimeout{60};
    size_t memoryBudgetBytes = 128 * 1024 * 1024;  // 128 MB
    bool enableTimedUnload = true;
    bool enableMemoryPressureEviction = true;

    static HotsetConfig Aggressive() {
        HotsetConfig c;
        c.mode = HotsetMode::DominantOnly;
        c.idleTimeout = std::chrono::seconds(30);
        c.memoryBudgetBytes = 64 * 1024 * 1024;
        return c;
    }
    static HotsetConfig Balanced() { return HotsetConfig{}; }
    static HotsetConfig Conservative() {
        HotsetConfig c;
        c.mode = HotsetMode::AllDecoders;
        c.idleTimeout = std::chrono::seconds(300);
        c.memoryBudgetBytes = 256 * 1024 * 1024;
        return c;
    }
};

/// Hotset statistics
struct HotsetStats {
    int totalDecoders = 0;
    int hotDecoders = 0;
    int coldDecoders = 0;
    int evictedDecoders = 0;
    size_t totalMemoryUsed = 0;
    size_t memoryBudget = 0;
    int loadOperations = 0;
    int unloadOperations = 0;

    double MemoryUtilization() const {
        return memoryBudget > 0 ? 100.0 * totalMemoryUsed / memoryBudget : 0.0;
    }
    bool IsUnderBudget() const { return totalMemoryUsed <= memoryBudget; }
};

/// Unload candidate for priority-based eviction
struct UnloadCandidate {
    std::string decoderName;
    double idleSeconds = 0.0;
    size_t memoryBytes = 0;
    int useCount = 0;
    double priority = 0.0;  // Higher = should unload first

    bool operator<(const UnloadCandidate& o) const { return priority < o.priority; }
};

/// Decoder hotset manager
class DecoderHotsetManager {
public:
    static DecoderHotsetManager Create(const HotsetConfig& config = HotsetConfig::Balanced()) {
        DecoderHotsetManager mgr;
        mgr.m_config = config;
        mgr.InitializeDecoders();
        return mgr;
    }

    // ─── Decoder Registration ────────────────────────────────────
    void RegisterDecoder(const std::string& name, const std::vector<std::string>& exts,
                         size_t footprintBytes = 4*1024*1024) {
        DecoderHotsetEntry entry;
        entry.decoderName = name;
        entry.extensions = exts;
        entry.memoryFootprintBytes = footprintBytes;
        entry.state = DecoderLoadState::Cold;
        m_decoders[name] = entry;
    }

    // ─── Load / Unload ───────────────────────────────────────────
    bool LoadDecoder(const std::string& name) {
        auto it = m_decoders.find(name);
        if (it == m_decoders.end()) return false;
        it->second.state = DecoderLoadState::Hot;
        it->second.loadedAt = std::chrono::steady_clock::now();
        it->second.lastUsed = it->second.loadedAt;
        m_stats.loadOperations++;
        return true;
    }

    bool UnloadDecoder(const std::string& name) {
        auto it = m_decoders.find(name);
        if (it == m_decoders.end()) return false;
        it->second.state = DecoderLoadState::Cold;
        m_stats.unloadOperations++;
        return true;
    }

    void MarkUsed(const std::string& name) {
        auto it = m_decoders.find(name);
        if (it != m_decoders.end()) {
            it->second.lastUsed = std::chrono::steady_clock::now();
            it->second.useCount++;
        }
    }

    // ─── Hotset Activation ───────────────────────────────────────
    void ActivateForExtensions(const std::vector<std::string>& extensions) {
        for (auto& [name, entry] : m_decoders) {
            bool needed = false;
            for (const auto& ext : extensions) {
                if (std::find(entry.extensions.begin(), entry.extensions.end(), ext) != entry.extensions.end()) {
                    needed = true;
                    break;
                }
            }
            if (needed && entry.state == DecoderLoadState::Cold) {
                LoadDecoder(name);
            }
        }
    }

    // ─── Idle Eviction ───────────────────────────────────────────
    std::vector<UnloadCandidate> FindIdleCandidates() const {
        std::vector<UnloadCandidate> candidates;
        for (const auto& [name, entry] : m_decoders) {
            if (entry.IsIdle(m_config.idleTimeout)) {
                UnloadCandidate c;
                c.decoderName = name;
                c.idleSeconds = entry.IdleSeconds();
                c.memoryBytes = entry.memoryFootprintBytes;
                c.useCount = entry.useCount;
                c.priority = c.idleSeconds * c.memoryBytes / (c.useCount + 1.0);
                candidates.push_back(c);
            }
        }
        std::sort(candidates.rbegin(), candidates.rend());
        return candidates;
    }

    int EvictIdleDecoders() {
        auto candidates = FindIdleCandidates();
        int evicted = 0;
        for (const auto& c : candidates) {
            UnloadDecoder(c.decoderName);
            evicted++;
        }
        m_stats.evictedDecoders += evicted;
        return evicted;
    }

    // ─── Statistics ──────────────────────────────────────────────
    HotsetStats GetStats() const {
        HotsetStats stats = m_stats;
        stats.totalDecoders = static_cast<int>(m_decoders.size());
        stats.memoryBudget = m_config.memoryBudgetBytes;
        stats.totalMemoryUsed = 0;
        stats.hotDecoders = 0;
        stats.coldDecoders = 0;
        for (const auto& [name, entry] : m_decoders) {
            if (entry.IsActive()) {
                stats.hotDecoders++;
                stats.totalMemoryUsed += entry.memoryFootprintBytes;
            } else {
                stats.coldDecoders++;
            }
        }
        return stats;
    }

    size_t DecoderCount() const { return m_decoders.size(); }
    const HotsetConfig& Config() const { return m_config; }

private:
    HotsetConfig m_config;
    std::map<std::string, DecoderHotsetEntry> m_decoders;
    HotsetStats m_stats;

    void InitializeDecoders() {
        RegisterDecoder("ImageDecoder", {".jpg",".jpeg",".png",".bmp",".gif"}, 2*1024*1024);
        RegisterDecoder("WebPDecoder", {".webp"}, 4*1024*1024);
        RegisterDecoder("HEIFDecoder", {".heif",".heic"}, 8*1024*1024);
        RegisterDecoder("JXLDecoder", {".jxl"}, 6*1024*1024);
        RegisterDecoder("AVIFDecoder", {".avif"}, 6*1024*1024);
        RegisterDecoder("RAWDecoder", {".cr2",".cr3",".arw",".nef",".dng"}, 16*1024*1024);
        RegisterDecoder("ArchiveDecoder", {".zip",".rar",".7z",".cbz",".cbr"}, 4*1024*1024);
        RegisterDecoder("VideoDecoder", {".mp4",".mkv",".avi",".mov"}, 12*1024*1024);
    }
};

}} // namespace ExplorerLens::Memory

