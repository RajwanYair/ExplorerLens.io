// UniversalLibraryManifest.h — Library Manifest with SPDX Attributions
// Copyright (c) 2026 ExplorerLens Project
//
// Generates a machine-readable manifest of all third-party libraries with SPDX
// license identifiers, copyright notices, and completeness validation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class SPDXLicense : uint8_t {
    MIT = 0, Apache2, BSD3, LGPL21, Zlib, MPL2, ISC, BSL1, Custom, Unknown
};

inline const char* SPDXLicenseToString(SPDXLicense lic) {
    switch (lic) {
        case SPDXLicense::MIT:     return "MIT";
        case SPDXLicense::Apache2: return "Apache-2.0";
        case SPDXLicense::BSD3:    return "BSD-3-Clause";
        case SPDXLicense::LGPL21:  return "LGPL-2.1-only";
        case SPDXLicense::Zlib:    return "Zlib";
        case SPDXLicense::MPL2:    return "MPL-2.0";
        case SPDXLicense::ISC:     return "ISC";
        case SPDXLicense::BSL1:    return "BSL-1.0";
        case SPDXLicense::Custom:  return "LicenseRef-Custom";
        default:                   return "NOASSERTION";
    }
}

struct ThirdPartyLib {
    std::string name;
    std::string version;
    SPDXLicense license = SPDXLicense::Unknown;
    std::string spdxId;
    std::string url;
    std::string copyright;
    uint64_t sizeBytes = 0;
    bool isStaticLinked = true;

    std::string GetSPDXId() const {
        return spdxId.empty() ? SPDXLicenseToString(license) : spdxId;
    }
};

struct LibraryManifest {
    std::string product;
    std::string version;
    std::string buildDate;
    std::vector<ThirdPartyLib> libs;
    uint64_t totalSize = 0;
    std::string spdxDocNamespace;

    void ComputeTotalSize() {
        totalSize = 0;
        for (const auto& lib : libs) totalSize += lib.sizeBytes;
    }
};

class UniversalLibraryManifest {
public:
    UniversalLibraryManifest()
        : m_includeDevDeps(false) {
        PopulateKnownLibraries();
    }

    ~UniversalLibraryManifest() = default;

    LibraryManifest Generate() const {
        LibraryManifest manifest;
        manifest.product = "ExplorerLens";
        manifest.version = "30.5.0";
        manifest.buildDate = __DATE__;
        manifest.libs = m_libraries;
        manifest.spdxDocNamespace = "https://spdx.org/spdxdocs/ExplorerLens-30.5.0";
        manifest.ComputeTotalSize();
        return manifest;
    }

    std::string ExportSPDX() const {
        auto manifest = Generate();
        std::string spdx = "SPDXVersion: SPDX-2.3\n";
        spdx += "DataLicense: CC0-1.0\n";
        spdx += "SPDXID: SPDXRef-DOCUMENT\n";
        spdx += "DocumentName: " + manifest.product + "\n";
        spdx += "DocumentNamespace: " + manifest.spdxDocNamespace + "\n\n";
        for (size_t i = 0; i < manifest.libs.size(); ++i) {
            const auto& lib = manifest.libs[i];
            spdx += "PackageName: " + lib.name + "\n";
            spdx += "SPDXID: SPDXRef-Package-" + std::to_string(i) + "\n";
            spdx += "PackageVersion: " + lib.version + "\n";
            spdx += "PackageDownloadLocation: " + (lib.url.empty() ? "NOASSERTION" : lib.url) + "\n";
            spdx += "PackageLicenseConcluded: " + lib.GetSPDXId() + "\n";
            spdx += "PackageCopyrightText: " + (lib.copyright.empty() ? "NOASSERTION" : lib.copyright) + "\n\n";
        }
        return spdx;
    }

    std::vector<std::string> GetAttributions() const {
        std::vector<std::string> attr;
        for (const auto& lib : m_libraries)
            attr.push_back(lib.name + " " + lib.version + " (" + lib.GetSPDXId() + ")");
        return attr;
    }

    std::string GetLicenseText(const std::string& libName) const {
        for (const auto& lib : m_libraries)
            if (lib.name == libName)
                return lib.copyright.empty() ? "See " + lib.url : lib.copyright;
        return "";
    }

    bool ValidateCompleteness() const {
        for (const auto& lib : m_libraries) {
            if (lib.name.empty() || lib.version.empty()) return false;
            if (lib.license == SPDXLicense::Unknown) return false;
        }
        return true;
    }

    void AddLibrary(ThirdPartyLib lib) { m_libraries.push_back(std::move(lib)); }
    size_t GetLibraryCount() const { return m_libraries.size(); }
    void SetIncludeDevDeps(bool include) { m_includeDevDeps = include; }

private:
    void PopulateKnownLibraries() {
        m_libraries = {
            {"zlib",       "1.3.1",  SPDXLicense::Zlib,   "Zlib",      "", "Jean-loup Gailly, Mark Adler", 131072, true},
            {"LZ4",        "1.10.0", SPDXLicense::BSD3,   "BSD-3-Clause","","Yann Collet", 98304, true},
            {"zstd",       "1.5.7",  SPDXLicense::BSD3,   "BSD-3-Clause","","Meta Platforms", 524288, true},
            {"libwebp",    "1.5.0",  SPDXLicense::BSD3,   "BSD-3-Clause","","Google LLC", 360448, true},
            {"libjxl",     "0.11.1", SPDXLicense::BSD3,   "BSD-3-Clause","","JPEG XL Contributors", 819200, true},
            {"libavif",    "1.3.0",  SPDXLicense::BSD3,   "BSD-3-Clause","","Alliance for Open Media", 262144, true},
            {"libheif",    "1.19.5", SPDXLicense::LGPL21, "LGPL-2.1-only","","Dirk Farin", 409600, true},
            {"LibRaw",     "0.21.3", SPDXLicense::LGPL21, "LGPL-2.1-only","","LibRaw LLC", 716800, true},
            {"MuPDF",      "1.24.11",SPDXLicense::Custom, "LicenseRef-MuPDF","","Artifex Software", 2097152, true},
            {"libarchive", "3.7.6",  SPDXLicense::BSD3,   "BSD-3-Clause","","Tim Kientzle", 458752, true},
        };
    }

    std::vector<ThirdPartyLib> m_libraries;
    bool m_includeDevDeps;
};

}} // namespace ExplorerLens::Engine
