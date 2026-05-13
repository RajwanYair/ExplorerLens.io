#pragma once
/**
 * @file SBOMGenerator.h
 * @brief Generates Software Bill of Materials (SBOM) in SPDX and CycloneDX formats.
 * @version 15.0.0
 * @date 2026-03-02
 *
 * Lists every external dependency shipped with ExplorerLens, including license,
 * version, supplier, and package URL. Outputs SPDX 2.3, CycloneDX 1.5, and
 * human-readable third-party notice documents.
 *
 * @note Header-only. Uses Windows API + C++20 standard library only.
 * @note UUID generation via CoCreateGuid (requires linking ole32.lib).
 *
 * @copyright (c) 2026 ExplorerLens Contributors. All rights reserved.
 */

#include <objbase.h>
#include <windows.h>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// @brief Generates SBOM documents for ExplorerLens and its dependencies.
class SBOMGeneratorEngine
{
  public:
    /// @brief Metadata for a single dependency.
    struct DependencyInfo
    {
        std::string name;
        std::string version;
        std::string license;
        std::string supplier;
        std::string downloadUrl;
        std::string sha256;
        std::string purl;  ///< Package URL (pkg:generic/name@version)
    };

    SBOMGeneratorEngine() noexcept
    {
        InitializeSRWLock(&m_lock);
        PopulateDefaults();
    }

    ~SBOMGeneratorEngine() = default;

    SBOMGeneratorEngine(const SBOMGeneratorEngine&) = delete;
    SBOMGeneratorEngine& operator=(const SBOMGeneratorEngine&) = delete;

    /// @brief Add a custom dependency entry.
    inline void AddDependency(DependencyInfo&& dep)
    {
        AcquireSRWLockExclusive(&m_lock);
        m_deps.emplace_back(std::move(dep));
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// @brief Return all registered dependencies.
    inline std::vector<DependencyInfo> GetAllDependencies() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto copy = m_deps;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return copy;
    }

    /// @brief Project metadata string.
    inline std::string GetProjectInfo() const
    {
        std::ostringstream os;
        os << "Project: ExplorerLens\n"
           << "Version: 15.0.0\n"
           << "Codename: Zenith\n"
           << "Supplier: ExplorerLens Contributors\n"
           << "URL: https://github.com/explorerlensteam/ExplorerLens\n"
           << "License: Proprietary\n";
        return os.str();
    }

    // -----------------------------------------------------------------
    //  SPDX 2.3 generation
    // -----------------------------------------------------------------

    /// @brief Generate an SPDX 2.3 document as a JSON-structured string.
    inline std::string GenerateSPDX() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto deps = m_deps;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        std::string uuid = GenerateUUID();
        std::string ts = GetISO8601Timestamp();

        std::ostringstream os;
        os << "{\n"
           << "  \"spdxVersion\": \"SPDX-2.3\",\n"
           << "  \"dataLicense\": \"CC0-1.0\",\n"
           << "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n"
           << "  \"name\": \"ExplorerLens-39.4.0\",\n"
           << "  \"documentNamespace\": \"https://explorerlensteam.github.io/spdx/" << uuid << "\",\n"
           << "  \"creationInfo\": {\n"
           << "    \"created\": \"" << ts << "\",\n"
           << "    \"creators\": [\"Tool: ExplorerLens-SBOMGenerator-39.4.0\"]\n"
           << "  },\n"
           << "  \"packages\": [\n";

        // Root package
        os << "    {\n"
           << "      \"SPDXID\": \"SPDXRef-Package-ExplorerLens\",\n"
           << "      \"name\": \"ExplorerLens\",\n"
           << "      \"versionInfo\": \"15.0.0\",\n"
           << "      \"supplier\": \"Organization: ExplorerLens Contributors\",\n"
           << "      \"downloadLocation\": \"https://github.com/explorerlensteam/ExplorerLens\",\n"
           << "      \"licenseConcluded\": \"LicenseRef-Proprietary\",\n"
           << "      \"licenseDeclared\": \"LicenseRef-Proprietary\",\n"
           << "      \"copyrightText\": \"(c) 2026 ExplorerLens Contributors\",\n"
           << "      \"primaryPackagePurpose\": \"APPLICATION\"\n"
           << "    }";

        for (size_t i = 0; i < deps.size(); ++i) {
            auto& d = deps[i];
            std::string spdxId = "SPDXRef-Package-" + SanitizeId(d.name);
            os << ",\n    {\n"
               << "      \"SPDXID\": \"" << spdxId << "\",\n"
               << "      \"name\": \"" << d.name << "\",\n"
               << "      \"versionInfo\": \"" << d.version << "\",\n"
               << "      \"supplier\": \"Organization: " << d.supplier << "\",\n"
               << "      \"downloadLocation\": \"" << d.downloadUrl << "\",\n"
               << "      \"licenseConcluded\": \"" << d.license << "\",\n"
               << "      \"licenseDeclared\": \"" << d.license << "\",\n"
               << "      \"copyrightText\": \"NOASSERTION\"";
            if (!d.sha256.empty()) {
                os << ",\n      \"checksums\": [{\"algorithm\": \"SHA256\", \"checksumValue\": \"" << d.sha256
                   << "\"}]";
            }
            if (!d.purl.empty()) {
                os << ",\n      \"externalRefs\": [{\"referenceCategory\": \"PACKAGE-MANAGER\", "
                   << "\"referenceType\": \"purl\", \"referenceLocator\": \"" << d.purl << "\"}]";
            }
            os << "\n    }";
        }

        os << "\n  ],\n";

        // Relationships
        os << "  \"relationships\": [\n";
        for (size_t i = 0; i < deps.size(); ++i) {
            std::string spdxId = "SPDXRef-Package-" + SanitizeId(deps[i].name);
            os << "    {\"spdxElementId\": \"SPDXRef-Package-ExplorerLens\", "
               << "\"relationshipType\": \"DEPENDS_ON\", "
               << "\"relatedSpdxElement\": \"" << spdxId << "\"}";
            if (i + 1 < deps.size())
                os << ",";
            os << "\n";
        }
        os << "  ]\n}\n";
        return os.str();
    }

    // -----------------------------------------------------------------
    //  CycloneDX 1.5 generation
    // -----------------------------------------------------------------

    /// @brief Generate a CycloneDX 1.5 document (XML-structured string).
    inline std::string GenerateCycloneDX() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto deps = m_deps;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        std::string uuid = GenerateUUID();
        std::string ts = GetISO8601Timestamp();

        std::ostringstream os;
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           << "<bom xmlns=\"http://cyclonedx.org/schema/bom/1.5\"\n"
           << "     version=\"1\" serialNumber=\"urn:uuid:" << uuid << "\">\n"
           << "  <metadata>\n"
           << "    <timestamp>" << ts << "</timestamp>\n"
           << "    <tools>\n"
           << "      <tool>\n"
           << "        <vendor>ExplorerLens</vendor>\n"
           << "        <name>SBOMGenerator</name>\n"
           << "        <version>15.0.0</version>\n"
           << "      </tool>\n"
           << "    </tools>\n"
           << "    <component type=\"application\">\n"
           << "      <name>ExplorerLens</name>\n"
           << "      <version>15.0.0</version>\n"
           << "    </component>\n"
           << "  </metadata>\n"
           << "  <components>\n";

        for (auto& d : deps) {
            os << "    <component type=\"library\">\n"
               << "      <name>" << EscapeXml(d.name) << "</name>\n"
               << "      <version>" << EscapeXml(d.version) << "</version>\n"
               << "      <licenses>\n"
               << "        <license><id>" << EscapeXml(d.license) << "</id></license>\n"
               << "      </licenses>\n";
            if (!d.purl.empty()) {
                os << "      <purl>" << EscapeXml(d.purl) << "</purl>\n";
            }
            if (!d.supplier.empty()) {
                os << "      <supplier><name>" << EscapeXml(d.supplier) << "</name></supplier>\n";
            }
            if (!d.sha256.empty()) {
                os << "      <hashes><hash alg=\"SHA-256\">" << d.sha256 << "</hash></hashes>\n";
            }
            os << "    </component>\n";
        }

        os << "  </components>\n"
           << "  <dependencies>\n"
           << "    <dependency ref=\"ExplorerLens\">\n";
        for (auto& d : deps) {
            os << "      <dependency ref=\"" << EscapeXml(d.name) << "\"/>\n";
        }
        os << "    </dependency>\n"
           << "  </dependencies>\n"
           << "</bom>\n";
        return os.str();
    }

    // -----------------------------------------------------------------
    //  Third-party notice
    // -----------------------------------------------------------------

    /// @brief Generate a human-readable third-party notice.
    inline std::string GenerateThirdPartyNotice() const
    {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        auto deps = m_deps;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));

        std::ostringstream os;
        os << "===============================================================\n"
           << "  ExplorerLens v15.0.0 — Third-Party Library Notices\n"
           << "===============================================================\n\n"
           << "This product includes software developed by third parties.\n"
           << "Below is a listing of each library along with its license.\n\n";

        for (size_t i = 0; i < deps.size(); ++i) {
            auto& d = deps[i];
            os << "---------------------------------------------------------------\n"
               << "  " << (i + 1) << ". " << d.name << " " << d.version << "\n"
               << "---------------------------------------------------------------\n"
               << "  License  : " << d.license << "\n"
               << "  Supplier : " << d.supplier << "\n"
               << "  URL      : " << d.downloadUrl << "\n";
            if (!d.sha256.empty()) {
                os << "  SHA-256  : " << d.sha256 << "\n";
            }
            os << "\n";
        }

        os << "===============================================================\n"
           << "  End of Third-Party Notices\n"
           << "===============================================================\n";
        return os.str();
    }

    // -----------------------------------------------------------------
    //  File I/O
    // -----------------------------------------------------------------

    /// @brief Save SBOM to file in the given format ("spdx" or "cyclonedx").
    inline bool SaveSBOM(const std::wstring& path, const std::string& format = "spdx") const
    {
        std::string content;
        if (format == "cyclonedx" || format == "cdx") {
            content = GenerateCycloneDX();
        } else {
            content = GenerateSPDX();
        }

        HANDLE hFile =
            CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
            return false;

        DWORD written = 0;
        BOOL ok = WriteFile(hFile, content.data(), static_cast<DWORD>(content.size()), &written, nullptr);
        CloseHandle(hFile);
        return ok && written == content.size();
    }

  private:
    SRWLOCK m_lock{};
    std::vector<DependencyInfo> m_deps;

    /// Populate built-in dependency list.
    inline void PopulateDefaults()
    {
        m_deps = {
            {"zlib", "1.3.1", "Zlib", "Jean-loup Gailly & Mark Adler", "https://zlib.net/", "",
             "pkg:generic/zlib@1.3.1"},

            {"LZ4", "1.10.0", "BSD-2-Clause", "Yann Collet", "https://github.com/lz4/lz4", "", "pkg:generic/lz4@1.10.0"},

            {"zstd", "1.5.7", "BSD-3-Clause", "Meta Platforms / Yann Collet", "https://github.com/facebook/zstd", "",
             "pkg:generic/zstd@1.5.7"},

            {"LZMA SDK", "26.00", "public-domain", "Igor Pavlov", "https://7-zip.org/sdk.html", "",
             "pkg:generic/lzma-sdk@26.00"},

            {"minizip-ng", "4.0.10", "Zlib", "Nathan Moinvaziri", "https://github.com/zlib-ng/minizip-ng", "",
             "pkg:generic/minizip-ng@4.0.10"},

            {"UnRAR", "7.2.2", "LicenseRef-UnRAR-restrictive", "Alexander L. Roshal",
             "https://www.rarlab.com/rar_add.htm", "", "pkg:generic/unrar@7.2.2"},

            {"libwebp", "1.5.0", "BSD-3-Clause", "Google LLC", "https://chromium.googlesource.com/webm/libwebp", "",
             "pkg:generic/libwebp@1.5.0"},

            {"libavif", "1.3.0", "BSD-2-Clause", "Alliance for Open Media", "https://github.com/AOMediaCodec/libavif",
             "", "pkg:generic/libavif@1.3.0"},

            {"libjxl", "0.11.1", "BSD-3-Clause", "libjxl Contributors", "https://github.com/libjxl/libjxl", "",
             "pkg:generic/libjxl@0.11.1"},

            {"libheif", "1.19.5", "LGPL-3.0-only", "Dirk Farin", "https://github.com/nickt/libheif", "",
             "pkg:generic/libheif@1.19.5"},

            {"libde265", "1.0.15", "LGPL-3.0-only", "Dirk Farin", "https://github.com/nickt/libde265", "",
             "pkg:generic/libde265@1.0.15"},

            {"LibRaw", "0.21.3", "LGPL-2.1-only OR CDDL-1.0", "LibRaw LLC", "https://www.libraw.org/", "",
             "pkg:generic/libraw@0.21.3"},

            {"dav1d", "1.4.3", "BSD-2-Clause", "VideoLAN", "https://code.videolan.org/videolan/dav1d", "",
             "pkg:generic/dav1d@1.4.3"},
        };
    }

    /// Generate a UUID string via CoCreateGuid.
    static inline std::string GenerateUUID()
    {
        GUID guid{};
        if (SUCCEEDED(CoCreateGuid(&guid))) {
            char buf[64]{};
            _snprintf_s(buf, _countof(buf), _TRUNCATE, "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid.Data1,
                        guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
            return std::string(buf);
        }
        return "00000000-0000-0000-0000-000000000000";
    }

    /// Get ISO-8601 timestamp.
    static inline std::string GetISO8601Timestamp()
    {
        SYSTEMTIME st{};
        GetSystemTime(&st);
        std::ostringstream os;
        os << std::setfill('0') << st.wYear << '-' << std::setw(2) << st.wMonth << '-' << std::setw(2) << st.wDay << 'T'
           << std::setw(2) << st.wHour << ':' << std::setw(2) << st.wMinute << ':' << std::setw(2) << st.wSecond << 'Z';
        return os.str();
    }

    /// Sanitize a name for use as SPDX ID (alphanumeric + hyphen).
    static inline std::string SanitizeId(const std::string& name)
    {
        std::string id;
        id.reserve(name.size());
        for (char c : name) {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-') {
                id += c;
            } else if (c == ' ' || c == '_') {
                id += '-';
            }
        }
        return id;
    }

    /// Escape XML special characters.
    static inline std::string EscapeXml(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 16);
        for (char c : s) {
            switch (c) {
                case '&':
                    out += "&amp;";
                    break;
                case '<':
                    out += "&lt;";
                    break;
                case '>':
                    out += "&gt;";
                    break;
                case '"':
                    out += "&quot;";
                    break;
                case '\'':
                    out += "&apos;";
                    break;
                default:
                    out += c;
                    break;
            }
        }
        return out;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
