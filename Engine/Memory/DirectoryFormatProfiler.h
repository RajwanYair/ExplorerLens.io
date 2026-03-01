#pragma once
//==============================================================================
// DirectoryFormatProfiler.h
// Lightweight extension histogram per folder, dominant-format detection,
// and single-format hot mode activation for memory optimization.
//==============================================================================

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <mutex>

namespace ExplorerLens {
namespace Memory {

/// Format family classification for memory budgeting
enum class FormatFamily : uint8_t {
    LightweightImage = 0, // JPEG, PNG, BMP, GIF, WebP
    ModernImage = 1, // HEIF, AVIF, JXL
    RawPhoto = 2, // CR3, ARW, NEF, DNG
    HDRImage = 3, // EXR, HDR, PFM
    Archive = 4, // ZIP, RAR, 7Z, CBZ, CBR
    Document = 5, // PDF, DOCX, EPUB
    Video = 6, // MP4, MKV, AVI, MOV
    Vector = 7, // SVG, AI, EPS
    Texture = 8, // DDS, KTX, BasisU
    Model3D = 9, // OBJ, STL, glTF
    Audio = 10, // MP3, FLAC, WAV
    Font = 11, // TTF, OTF, WOFF
    Unknown = 255
};

/// Extension histogram entry
struct ExtensionCount {
    std::string extension; // Lowercase, including dot (e.g., ".jpg")
    int count = 0;
    FormatFamily family = FormatFamily::Unknown;
    double ratio = 0.0; // Fraction of total files
};

/// Directory profile result
struct DirectoryProfile {
    std::string directoryPath;
    int totalFiles = 0;
    int supportedFiles = 0;
    int unsupportedFiles = 0;
    std::vector<ExtensionCount> histogram;
    FormatFamily dominantFamily = FormatFamily::Unknown;
    double dominantRatio = 0.0;
    bool isSingleFormatMode = false; // True when dominant >= 0.8
    std::chrono::milliseconds scanDuration{ 0 };

    bool HasDominantFormat() const { return dominantRatio >= 0.8; }

    std::string DominantExtension() const {
        if (histogram.empty()) return "";
        return histogram.front().extension;
    }

    std::vector<std::string> ActiveExtensions(int topN = 5) const {
        std::vector<std::string> result;
        int n = (std::min)(topN, static_cast<int>(histogram.size()));
        for (int i = 0; i < n; i++) {
            result.push_back(histogram[i].extension);
        }
        return result;
    }
};

/// Memory budget per format family
struct FamilyMemoryBudget {
    FormatFamily family;
    size_t decoderFootprintBytes; // Memory used by loaded decoder
    size_t perThumbnailBytes; // Average per-thumbnail memory
    size_t maxWorkingSetBytes; // Cap for this format family
    int maxConcurrentDecodes;

    static FamilyMemoryBudget LightweightImage() {
        return { FormatFamily::LightweightImage, 2 * 1024 * 1024, 256 * 1024, 32 * 1024 * 1024, 8 };
    }
    static FamilyMemoryBudget ModernImage() {
        return { FormatFamily::ModernImage, 8 * 1024 * 1024, 512 * 1024, 64 * 1024 * 1024, 4 };
    }
    static FamilyMemoryBudget RawPhoto() {
        return { FormatFamily::RawPhoto, 16 * 1024 * 1024, 2 * 1024 * 1024, 128 * 1024 * 1024, 2 };
    }
    static FamilyMemoryBudget HDRImage() {
        return { FormatFamily::HDRImage, 12 * 1024 * 1024, 1 * 1024 * 1024, 96 * 1024 * 1024, 2 };
    }
    static FamilyMemoryBudget Archive() {
        return { FormatFamily::Archive, 4 * 1024 * 1024, 512 * 1024, 48 * 1024 * 1024, 4 };
    }
};

/// Directory format profiler engine
class DirectoryFormatProfiler {
public:
    // ─── Extension → Family Mapping ──────────────────────────────
    FormatFamily ClassifyExtension(const std::string& ext) const {
        auto it = m_familyMap.find(ext);
        return it != m_familyMap.end() ? it->second : FormatFamily::Unknown;
    }

    // ─── Profile Directory ───────────────────────────────────────
    DirectoryProfile ProfileDirectory(const std::string& dirPath,
        const std::vector<std::string>& files) const {
        DirectoryProfile profile;
        profile.directoryPath = dirPath;
        profile.totalFiles = static_cast<int>(files.size());

        std::map<std::string, int> counts;
        for (const auto& file : files) {
            std::string ext = ExtractExtension(file);
            if (!ext.empty()) {
                counts[ext]++;
                if (ClassifyExtension(ext) != FormatFamily::Unknown) {
                    profile.supportedFiles++;
                }
                else {
                    profile.unsupportedFiles++;
                }
            }
        }

        for (const auto& [ext, count] : counts) {
            ExtensionCount ec;
            ec.extension = ext;
            ec.count = count;
            ec.family = ClassifyExtension(ext);
            ec.ratio = profile.totalFiles > 0 ? static_cast<double>(count) / profile.totalFiles : 0.0;
            profile.histogram.push_back(ec);
        }

        // Sort by count descending
        std::sort(profile.histogram.begin(), profile.histogram.end(),
            [](const ExtensionCount& a, const ExtensionCount& b) {
                return a.count > b.count;
            });

        // Determine dominant format family
        if (!profile.histogram.empty()) {
            profile.dominantFamily = profile.histogram.front().family;
            // Calculate family ratio (sum all extensions in dominant family)
            int familyCount = 0;
            for (const auto& ec : profile.histogram) {
                if (ec.family == profile.dominantFamily) familyCount += ec.count;
            }
            profile.dominantRatio = profile.totalFiles > 0
                ? static_cast<double>(familyCount) / profile.totalFiles : 0.0;
            profile.isSingleFormatMode = profile.dominantRatio >= 0.8;
        }

        return profile;
    }

    // ─── Memory Budget Selection ─────────────────────────────────
    FamilyMemoryBudget GetBudget(FormatFamily family) const {
        switch (family) {
        case FormatFamily::LightweightImage: return FamilyMemoryBudget::LightweightImage();
        case FormatFamily::ModernImage: return FamilyMemoryBudget::ModernImage();
        case FormatFamily::RawPhoto: return FamilyMemoryBudget::RawPhoto();
        case FormatFamily::HDRImage: return FamilyMemoryBudget::HDRImage();
        case FormatFamily::Archive: return FamilyMemoryBudget::Archive();
        default: return FamilyMemoryBudget::ModernImage(); // Fallback
        }
    }

    size_t FamilyMapSize() const { return m_familyMap.size(); }

    static DirectoryFormatProfiler Create() {
        DirectoryFormatProfiler p;
        p.InitializeFamilyMap();
        return p;
    }

private:
    std::map<std::string, FormatFamily> m_familyMap;

    std::string ExtractExtension(const std::string& path) const {
        auto pos = path.rfind('.');
        if (pos == std::string::npos || pos == path.length() - 1) return "";
        std::string ext = path.substr(pos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    void InitializeFamilyMap() {
        // Lightweight images
        for (auto e : { ".jpg",".jpeg",".png",".bmp",".gif",".webp",".tga",".ppm",".pgm",".ico" })
            m_familyMap[e] = FormatFamily::LightweightImage;
        // Modern images
        for (auto e : { ".heif",".heic",".avif",".jxl",".qoi" })
            m_familyMap[e] = FormatFamily::ModernImage;
        // RAW photos
        for (auto e : { ".cr2",".cr3",".arw",".nef",".dng",".orf",".rw2",".raf",".gpr",".pef" })
            m_familyMap[e] = FormatFamily::RawPhoto;
        // HDR images
        for (auto e : { ".exr",".hdr",".pfm",".psd",".psb" })
            m_familyMap[e] = FormatFamily::HDRImage;
        // Archives
        for (auto e : { ".zip",".rar",".7z",".cbz",".cbr",".cb7",".cbt",".tar",".gz",".xz" })
            m_familyMap[e] = FormatFamily::Archive;
        // Documents
        for (auto e : { ".pdf",".epub",".docx",".xlsx",".pptx" })
            m_familyMap[e] = FormatFamily::Document;
        // Video
        for (auto e : { ".mp4",".mkv",".avi",".mov",".wmv",".webm",".flv" })
            m_familyMap[e] = FormatFamily::Video;
        // Vector
        for (auto e : { ".svg",".svgz",".ai",".eps" })
            m_familyMap[e] = FormatFamily::Vector;
        // Texture
        for (auto e : { ".dds",".ktx",".ktx2",".basis" })
            m_familyMap[e] = FormatFamily::Texture;
        // 3D Model
        for (auto e : { ".obj",".stl",".gltf",".glb",".fbx",".3ds" })
            m_familyMap[e] = FormatFamily::Model3D;
        // Audio
        for (auto e : { ".mp3",".flac",".wav",".ogg",".aac",".wma" })
            m_familyMap[e] = FormatFamily::Audio;
        // Font
        for (auto e : { ".ttf",".otf",".woff",".woff2" })
            m_familyMap[e] = FormatFamily::Font;
    }
};

}
} // namespace ExplorerLens::Memory
