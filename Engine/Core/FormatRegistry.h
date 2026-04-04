//==============================================================================
// ExplorerLens Engine — Format Registry Refactor
// Replaces #define LENSTYPE with enum class FormatType.
// Central registry mapping extension → type → decoder → shell registration.
//==============================================================================
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Strongly-typed format identifier replacing legacy #define LENSTYPE int
enum class FormatType : uint16_t {
    Unknown = 0,
    ZIP = 1,
    CBZ = 2,
    CBR = 3,
    CB7 = 4,
    EPUB = 5,
    SevenZip = 6,
    RAR = 7,
    TAR = 8,
    GZIP = 9,
    BZIP2 = 10,
    XZ = 11,
    ZSTD = 12,
    CBT = 13,
    LZMA = 14,
    LZ4 = 21,
    Video = 18,
    PDF = 19,
    Audio = 20,
    ISO = 22,
    CHM = 23,
    CAB = 24,
    CPIO = 25,
    DEB = 26,
    XAR = 27,
    AR = 28,
    WebP = 40,
    AVIF = 41,
    HEIC = 42,
    HEIF = 43,
    JXL = 44,
    PSD = 48,
    DDS = 49,
    HDR = 50,
    EXR = 51,
    TGA = 52,
    ICO = 53,
    QOI = 54,
    SVG = 55,
    BMP = 56,
    GIF = 57,
    TIFF = 58,
    JPEG = 59,
    PNG = 60,
    RAW = 47,
    PPM = 61,
    PGM = 62,
    PBM = 63,
    PNM = 64,
    PAM = 65,
    PFM = 66,
    Document = 70,
    Font = 71,
    Model = 72,
    EBook = 73,
    DJVU = 74,
    WMF = 82,
    EMF = 83,
    PCX = 84,
    Farbfeld = 85,
    JP2 = 86,
    EPS = 87,
    KTX = 88,
    VTF = 89,
    ORA = 90,
    XCF = 91,
    SGI = 92,
    XPM = 93,
    DICOM = 94,
    FITS = 95,
    Geospatial = 96,
    DPX = 97,
    Cineon = 98,
    Markdown = 100,
    SourceCode = 101,
    CSV = 102,
    JSON = 103,
    Notebook = 104,
    FLIF = 110,
    BPG = 111,
    ThreeMF = 120,
    USD = 121,
    MAX_VALUE = 255
};

/// Category groupings
enum class FormatCategory : uint8_t {
    Unknown,
    Archive,
    Image,
    ModernImage,
    ProfessionalImage,
    Vector,
    RAWPhoto,
    Video,
    Audio,
    Document,
    PDF,
    Font,
    Model3D,
    EBook,
    Scientific,
    GameTexture,
    CreativeSuite,
    TextPreview,
    Data
};

/// Single format entry in the registry
struct FormatEntry
{
    FormatType type = FormatType::Unknown;
    FormatCategory category = FormatCategory::Unknown;
    std::wstring primaryExt;   // e.g. L".webp"
    std::wstring description;  // e.g. L"WebP Image"
    std::wstring decoderName;  // e.g. L"WebPDecoder"
    bool shellRegistered = false;
    bool hasDecoder = false;
    bool hasLibrary = false;

    // Convenience
    bool IsFullySupported() const
    {
        return hasDecoder && hasLibrary;
    }
};

/// Central format registry — singleton
class FormatRegistry
{
  public:
    static FormatRegistry& Instance()
    {
        static FormatRegistry s_instance;
        return s_instance;
    }

    /// Register a format entry
    void Register(const FormatEntry& entry)
    {
        m_entries[entry.type] = entry;
        m_extensionMap[entry.primaryExt] = entry.type;
    }

    /// Register additional extensions for an existing type
    void RegisterAlias(const std::wstring& ext, FormatType type)
    {
        m_extensionMap[ext] = type;
    }

    /// Look up type by extension (case-insensitive key expected)
    FormatType LookupByExtension(const std::wstring& ext) const
    {
        auto it = m_extensionMap.find(ext);
        return (it != m_extensionMap.end()) ? it->second : FormatType::Unknown;
    }

    /// Get entry by type
    const FormatEntry* GetEntry(FormatType type) const
    {
        auto it = m_entries.find(type);
        return (it != m_entries.end()) ? &it->second : nullptr;
    }

    /// Count registered formats
    size_t Count() const
    {
        return m_entries.size();
    }

    /// Count shell-registered formats
    size_t ShellRegisteredCount() const
    {
        size_t n = 0;
        for (auto& [k, v] : m_entries)
            if (v.shellRegistered)
                n++;
        return n;
    }

    /// Get all entries
    const std::unordered_map<FormatType, FormatEntry>& Entries() const
    {
        return m_entries;
    }

    /// Category name helper
    static const wchar_t* CategoryName(FormatCategory c)
    {
        switch (c) {
            case FormatCategory::Archive:
                return L"Archive";
            case FormatCategory::Image:
                return L"Image";
            case FormatCategory::ModernImage:
                return L"ModernImage";
            case FormatCategory::ProfessionalImage:
                return L"ProfessionalImage";
            case FormatCategory::Vector:
                return L"Vector";
            case FormatCategory::RAWPhoto:
                return L"RAWPhoto";
            case FormatCategory::Video:
                return L"Video";
            case FormatCategory::Audio:
                return L"Audio";
            case FormatCategory::Document:
                return L"Document";
            case FormatCategory::PDF:
                return L"PDF";
            case FormatCategory::Font:
                return L"Font";
            case FormatCategory::Model3D:
                return L"3DModel";
            case FormatCategory::EBook:
                return L"EBook";
            case FormatCategory::Scientific:
                return L"Scientific";
            case FormatCategory::GameTexture:
                return L"GameTexture";
            case FormatCategory::CreativeSuite:
                return L"CreativeSuite";
            case FormatCategory::TextPreview:
                return L"TextPreview";
            case FormatCategory::Data:
                return L"Data";
            default:
                return L"Unknown";
        }
    }

    /// FormatType name helper
    static const wchar_t* TypeName(FormatType t)
    {
        switch (t) {
            case FormatType::ZIP:
                return L"ZIP";
            case FormatType::WebP:
                return L"WebP";
            case FormatType::AVIF:
                return L"AVIF";
            case FormatType::HEIC:
                return L"HEIC";
            case FormatType::JXL:
                return L"JPEG XL";
            case FormatType::PSD:
                return L"PSD";
            case FormatType::RAW:
                return L"RAW";
            case FormatType::PDF:
                return L"PDF";
            case FormatType::DPX:
                return L"DPX";
            case FormatType::Cineon:
                return L"Cineon";
            case FormatType::DICOM:
                return L"DICOM";
            case FormatType::FITS:
                return L"FITS";
            case FormatType::Markdown:
                return L"Markdown";
            case FormatType::SourceCode:
                return L"SourceCode";
            default:
                return L"Unknown";
        }
    }

    /// Validate that extension map is consistent with entries
    struct ValidationResult
    {
        bool valid = true;
        size_t totalEntries = 0;
        size_t orphanedExtensions = 0;
        size_t missingPrimaryExt = 0;
    };

    ValidationResult Validate() const
    {
        ValidationResult r;
        r.totalEntries = m_entries.size();
        for (auto& [ext, type] : m_extensionMap) {
            if (m_entries.find(type) == m_entries.end())
                r.orphanedExtensions++;
        }
        for (auto& [type, entry] : m_entries) {
            if (entry.primaryExt.empty())
                r.missingPrimaryExt++;
        }
        r.valid = (r.orphanedExtensions == 0 && r.missingPrimaryExt == 0);
        return r;
    }

  private:
    FormatRegistry() = default;
    std::unordered_map<FormatType, FormatEntry> m_entries;
    std::unordered_map<std::wstring, FormatType> m_extensionMap;
};

}  // namespace Engine
}  // namespace ExplorerLens
