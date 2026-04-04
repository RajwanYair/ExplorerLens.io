#pragma once
//==============================================================================
// ExplorerLens — Duplicate Detection & Perceptual Hashing
// pHash/dHash computation, Hamming distance API, duplicate grouping,
// similarity search, CSV/JSON export for external tools.
//==============================================================================

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens::Engine::Utils {

//------------------------------------------------------------------------------
// Hash Types
//------------------------------------------------------------------------------
enum class PerceptualHashAlgo : uint8_t {
    pHash = 0,  // Perceptual hash — DCT-based, rotation-tolerant
    dHash,      // Difference hash — gradient-based, very fast
    aHash,      // Average hash — simplest, brightness-based
    wHash       // Wavelet hash — Haar wavelet, good compression tolerance
};

inline const char* HashAlgorithmName(PerceptualHashAlgo algo)
{
    switch (algo) {
        case PerceptualHashAlgo::pHash:
            return "pHash (Perceptual)";
        case PerceptualHashAlgo::dHash:
            return "dHash (Difference)";
        case PerceptualHashAlgo::aHash:
            return "aHash (Average)";
        case PerceptualHashAlgo::wHash:
            return "wHash (Wavelet)";
        default:
            return "Unknown";
    }
}

//------------------------------------------------------------------------------
// Perceptual Hash Value — 64-bit fingerprint
//------------------------------------------------------------------------------
struct PerceptualHash
{
    uint64_t value = 0;
    PerceptualHashAlgo algorithm = PerceptualHashAlgo::pHash;
    uint32_t imageWidth = 0;
    uint32_t imageHeight = 0;

    // Hex string representation
    std::string ToHex() const
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << value;
        return oss.str();
    }

    // Hamming distance to another hash
    uint32_t HammingDistance(const PerceptualHash& other) const
    {
        uint64_t xorVal = value ^ other.value;
        uint32_t dist = 0;
        while (xorVal) {
            dist += xorVal & 1;
            xorVal >>= 1;
        }
        return dist;
    }

    // Similarity percentage (0-100)
    double Similarity(const PerceptualHash& other) const
    {
        uint32_t dist = HammingDistance(other);
        return (1.0 - dist / 64.0) * 100.0;
    }

    bool IsExactMatch(const PerceptualHash& other) const
    {
        return value == other.value;
    }

    bool IsSimilar(const PerceptualHash& other, uint32_t threshold = 10) const
    {
        return HammingDistance(other) <= threshold;
    }

    bool IsValid() const
    {
        return value != 0;
    }
};

//------------------------------------------------------------------------------
// Hash Computation Parameters
//------------------------------------------------------------------------------
struct HashComputeParams
{
    PerceptualHashAlgo algorithm = PerceptualHashAlgo::pHash;
    uint32_t resizeWidth = 32;  // pHash uses 32x32 for DCT
    uint32_t resizeHeight = 32;
    bool convertToGrayscale = true;
    bool normalizeIntensity = true;

    static HashComputeParams ForPHash()
    {
        return {PerceptualHashAlgo::pHash, 32, 32, true, true};
    }

    static HashComputeParams ForDHash()
    {
        return {PerceptualHashAlgo::dHash, 9, 8, true, false};  // 9x8 → 8x8 differences
    }

    static HashComputeParams ForAHash()
    {
        return {PerceptualHashAlgo::aHash, 8, 8, true, true};
    }

    static HashComputeParams ForWHash()
    {
        return {PerceptualHashAlgo::wHash, 8, 8, true, true};
    }
};

//------------------------------------------------------------------------------
// Hash Database Entry — stored alongside thumbnail in cache DB
//------------------------------------------------------------------------------
struct HashEntry
{
    std::string filePath;
    PerceptualHash pHash;       // Primary perceptual hash
    PerceptualHash dHash;       // Lightweight difference hash
    uint64_t fileSize = 0;      // For quick pre-filter
    uint64_t modifiedTime = 0;  // File modification timestamp
    std::string formatType;     // "JPEG", "PNG", "HEIF", etc.

    bool HasPHash() const
    {
        return pHash.IsValid();
    }
    bool HasDHash() const
    {
        return dHash.IsValid();
    }
    bool HasBothHashes() const
    {
        return HasPHash() && HasDHash();
    }
};

//------------------------------------------------------------------------------
// Similarity Match Result
//------------------------------------------------------------------------------
struct SimilarityMatch
{
    std::string filePath;
    PerceptualHash hash;
    uint32_t hammingDistance = 0;
    double similarityPct = 0;
    uint64_t fileSize = 0;

    bool operator<(const SimilarityMatch& other) const
    {
        return hammingDistance < other.hammingDistance;
    }
};

//------------------------------------------------------------------------------
// Duplicate Group — cluster of visually similar images
//------------------------------------------------------------------------------
struct PerceptualDupGroup
{
    uint32_t groupId = 0;
    std::vector<SimilarityMatch> members;
    PerceptualHash representativeHash;  // Hash of the "best" member

    uint32_t Size() const
    {
        return static_cast<uint32_t>(members.size());
    }
    bool HasDuplicates() const
    {
        return members.size() > 1;
    }

    uint64_t TotalFileSize() const
    {
        uint64_t total = 0;
        for (auto& m : members)
            total += m.fileSize;
        return total;
    }

    uint64_t WastedSpace() const
    {
        if (members.empty())
            return 0;
        // All but the largest file is "wasted"
        uint64_t maxSize = 0;
        uint64_t total = 0;
        for (auto& m : members) {
            total += m.fileSize;
            if (m.fileSize > maxSize)
                maxSize = m.fileSize;
        }
        return total - maxSize;
    }
};

//------------------------------------------------------------------------------
// Hamming Distance Search API
//------------------------------------------------------------------------------
class SimilaritySearchEngine
{
  public:
    void AddHash(const std::string& path, const PerceptualHash& hash, uint64_t fileSize = 0)
    {
        m_entries.push_back({path, hash, fileSize});
    }

    // Find all hashes within threshold Hamming distance
    std::vector<SimilarityMatch> FindSimilar(const PerceptualHash& query, uint32_t threshold = 10) const
    {
        std::vector<SimilarityMatch> results;
        for (auto& entry : m_entries) {
            uint32_t dist = query.HammingDistance(entry.hash);
            if (dist <= threshold) {
                results.push_back({entry.path, entry.hash, dist, query.Similarity(entry.hash), entry.fileSize});
            }
        }
        std::sort(results.begin(), results.end());
        return results;
    }

    // Find exact matches (distance = 0)
    std::vector<SimilarityMatch> FindExact(const PerceptualHash& query) const
    {
        return FindSimilar(query, 0);
    }

    // Group all entries into duplicate clusters
    std::vector<PerceptualDupGroup> FindAllDuplicates(uint32_t threshold = 10) const
    {
        std::vector<PerceptualDupGroup> groups;
        std::vector<bool> visited(m_entries.size(), false);
        uint32_t groupId = 1;

        for (size_t i = 0; i < m_entries.size(); ++i) {
            if (visited[i])
                continue;
            PerceptualDupGroup group;
            group.groupId = groupId++;
            group.representativeHash = m_entries[i].hash;
            group.members.push_back({m_entries[i].path, m_entries[i].hash, 0, 100.0, m_entries[i].fileSize});
            visited[i] = true;

            for (size_t j = i + 1; j < m_entries.size(); ++j) {
                if (visited[j])
                    continue;
                uint32_t dist = m_entries[i].hash.HammingDistance(m_entries[j].hash);
                if (dist <= threshold) {
                    group.members.push_back({m_entries[j].path, m_entries[j].hash, dist,
                                             m_entries[i].hash.Similarity(m_entries[j].hash), m_entries[j].fileSize});
                    visited[j] = true;
                }
            }

            if (group.HasDuplicates()) {
                groups.push_back(std::move(group));
            }
        }
        return groups;
    }

    uint32_t EntryCount() const
    {
        return static_cast<uint32_t>(m_entries.size());
    }
    void Clear()
    {
        m_entries.clear();
    }

  private:
    struct SearchEntry
    {
        std::string path;
        PerceptualHash hash;
        uint64_t fileSize;
    };
    std::vector<SearchEntry> m_entries;
};

//------------------------------------------------------------------------------
// Scan Results & Reporting
//------------------------------------------------------------------------------
struct DuplicateScanResult
{
    uint32_t totalFiles = 0;
    uint32_t filesHashed = 0;
    uint32_t hashErrors = 0;
    uint32_t duplicateGroups = 0;
    uint32_t totalDuplicates = 0;  // Files that are duplicates (excluding originals)
    uint64_t wastedBytes = 0;
    double scanTimeMs = 0;
    PerceptualHashAlgo algorithm = PerceptualHashAlgo::pHash;
    uint32_t threshold = 10;

    double DuplicateRate() const
    {
        return (filesHashed > 0) ? (totalDuplicates * 100.0 / filesHashed) : 0;
    }

    std::string WastedSizeHuman() const
    {
        if (wastedBytes < 1024)
            return std::to_string(wastedBytes) + " B";
        if (wastedBytes < 1048576)
            return std::to_string(wastedBytes / 1024) + " KB";
        if (wastedBytes < 1073741824)
            return std::to_string(wastedBytes / 1048576) + " MB";
        return std::to_string(wastedBytes / 1073741824) + " GB";
    }

    std::string Summary() const
    {
        return std::to_string(totalDuplicates) + " duplicates in " + std::to_string(duplicateGroups)
               + " groups, wasting " + WastedSizeHuman() + " (" + std::to_string(static_cast<int>(DuplicateRate()))
               + "% duplicate rate)";
    }
};

//------------------------------------------------------------------------------
// Export Formats
//------------------------------------------------------------------------------
enum class HashExportFormat : uint8_t {
    CSV = 0,
    JSON,
    Text
};

inline const char* ExportFormatName(HashExportFormat fmt)
{
    switch (fmt) {
        case HashExportFormat::CSV:
            return "CSV";
        case HashExportFormat::JSON:
            return "JSON";
        case HashExportFormat::Text:
            return "Text";
        default:
            return "Unknown";
    }
}

// Generate CSV report from duplicate groups
inline std::string ExportDuplicatesCSV(const std::vector<PerceptualDupGroup>& groups)
{
    std::ostringstream csv;
    csv << "GroupID,FilePath,Hash,HammingDistance,Similarity%,FileSize\n";
    for (auto& g : groups) {
        for (auto& m : g.members) {
            csv << g.groupId << ","
                << "\"" << m.filePath << "\"," << m.hash.ToHex() << "," << m.hammingDistance << "," << std::fixed
                << std::setprecision(1) << m.similarityPct << "," << m.fileSize << "\n";
        }
    }
    return csv.str();
}

// Generate JSON report
inline std::string ExportDuplicatesJSON(const std::vector<PerceptualDupGroup>& groups)
{
    std::ostringstream json;
    json << "{\n \"groups\": [\n";
    for (size_t gi = 0; gi < groups.size(); ++gi) {
        auto& g = groups[gi];
        json << " {\n \"groupId\": " << g.groupId << ",\n \"members\": [\n";
        for (size_t mi = 0; mi < g.members.size(); ++mi) {
            auto& m = g.members[mi];
            json << " {\"path\": \"" << m.filePath << "\", \"hash\": \"" << m.hash.ToHex()
                 << "\", \"distance\": " << m.hammingDistance << ", \"similarity\": " << std::fixed
                 << std::setprecision(1) << m.similarityPct << ", \"size\": " << m.fileSize << "}";
            if (mi < g.members.size() - 1)
                json << ",";
            json << "\n";
        }
        json << " ]\n }";
        if (gi < groups.size() - 1)
            json << ",";
        json << "\n";
    }
    json << " ]\n}\n";
    return json.str();
}

//------------------------------------------------------------------------------
// Duplicate Detection Configuration
//------------------------------------------------------------------------------
struct DuplicateDetectionConfig
{
    PerceptualHashAlgo primaryAlgorithm = PerceptualHashAlgo::pHash;
    bool computeDHash = true;           // Also compute dHash for fast pre-filter
    uint32_t similarityThreshold = 10;  // Max Hamming distance for "similar"
    uint32_t exactThreshold = 0;        // Hamming distance for "exact" match
    bool skipSmallFiles = true;
    uint64_t minFileSize = 1024;  // Skip files < 1 KB
    bool skipLargeFiles = false;
    uint64_t maxFileSize = 0;  // 0 = no limit
    uint32_t maxFilesPerScan = 100000;
    HashExportFormat exportFormat = HashExportFormat::CSV;

    static DuplicateDetectionConfig Default()
    {
        return {};
    }

    static DuplicateDetectionConfig FastScan()
    {
        DuplicateDetectionConfig c;
        c.primaryAlgorithm = PerceptualHashAlgo::dHash;
        c.computeDHash = false;
        c.similarityThreshold = 5;
        return c;
    }

    static DuplicateDetectionConfig Strict()
    {
        DuplicateDetectionConfig c;
        c.similarityThreshold = 3;
        return c;
    }
};

}  // namespace ExplorerLens::Engine::Utils
