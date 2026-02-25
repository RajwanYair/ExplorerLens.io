// SBOMGenerator.h — Software Bill of Materials Generator
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 385
// Copyright (c) 2026 ExplorerLens Project
//
// Generates SPDX and CycloneDX SBOM documents for ExplorerLens, listing
// all components, libraries, licenses, and build tool versions.

#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// SBOM output format for generated documents
enum class SBOMOutputFormat : uint8_t {
  SPDX_JSON = 0,    ///< SPDX 2.3 JSON
  CycloneDX = 1,    ///< CycloneDX 1.5 JSON
  CSV = 2,          ///< Simple CSV for audits
  SPDX = SPDX_JSON, ///< Alias for SPDX_JSON
  COUNT = 3
};

/// License type
enum class LicenseType : uint8_t {
  MIT = 0,
  BSD2Clause = 1,
  BSD3Clause = 2,
  Apache2 = 3,
  LGPL21 = 4,
  Zlib = 5,
  Proprietary = 6,
  PublicDomain = 7,
  ISC = 8,
  MPL2 = 9,
  Custom = 10
};

/// Component entry for SBOM
struct SBOMComponent {
  const char *name = nullptr;        ///< e.g., "zlib"
  const char *version = nullptr;     ///< e.g., "1.3.1"
  const char *supplier = nullptr;    ///< e.g., "Jean-loup Gailly & Mark Adler"
  const char *downloadUrl = nullptr; ///< Source download URL
  LicenseType license = LicenseType::MIT;
  const char *licenseSpdxId = nullptr; ///< e.g., "Zlib"
  const char *cpe = nullptr;           ///< CPE 2.3 identifier
  const char *purl = nullptr;          ///< Package URL
  bool isDirectDependency = true;
  bool isModified = false; ///< Was source modified?
};

/// SBOM generator
class SBOMGenerator {
public:
  static SBOMGenerator &Instance() {
    static SBOMGenerator inst;
    return inst;
  }

  /// Component registry
  static constexpr uint32_t COMPONENT_COUNT = 18;

  static const SBOMComponent &GetComponent(uint32_t index) {
    static const SBOMComponent components[] = {
        {"zlib", "1.3.1", "Jean-loup Gailly", "https://zlib.net",
         LicenseType::Zlib, "Zlib", nullptr, "pkg:github/madler/zlib@1.3.1",
         true, false},
        {"lz4", "1.10.0", "Yann Collet", "https://github.com/lz4/lz4",
         LicenseType::BSD2Clause, "BSD-2-Clause", nullptr,
         "pkg:github/lz4/lz4@1.10.0", true, false},
        {"zstd", "1.5.7", "Meta / Yann Collet",
         "https://github.com/facebook/zstd", LicenseType::BSD3Clause,
         "BSD-3-Clause", nullptr, "pkg:github/facebook/zstd@1.5.7", true,
         false},
        {"lzma-sdk", "26.00", "Igor Pavlov", "https://7-zip.org/sdk.html",
         LicenseType::PublicDomain, "LicenseRef-LZMA-SDK", nullptr, nullptr,
         true, false},
        {"minizip-ng", "4.0.10", "Nathan Moinvaziri",
         "https://github.com/zlib-ng/minizip-ng", LicenseType::Zlib, "Zlib",
         nullptr, "pkg:github/zlib-ng/minizip-ng@4.0.10", true, false},
        {"unrar", "7.2.2", "Alexander Roshal", "https://www.rarlab.com",
         LicenseType::Custom, "LicenseRef-UnRAR", nullptr, nullptr, true,
         false},
        {"libwebp", "1.5.0", "Google",
         "https://chromium.googlesource.com/webm/libwebp",
         LicenseType::BSD3Clause, "BSD-3-Clause", nullptr,
         "pkg:github/nicedoc/webp@1.5.0", true, false},
        {"libavif", "1.3.0", "AOMedia",
         "https://github.com/AOMediaCodec/libavif", LicenseType::BSD2Clause,
         "BSD-2-Clause", nullptr, "pkg:github/AOMediaCodec/libavif@1.3.0", true,
         false},
        {"libjxl", "0.11.1", "Google", "https://github.com/libjxl/libjxl",
         LicenseType::BSD3Clause, "BSD-3-Clause", nullptr,
         "pkg:github/libjxl/libjxl@0.11.1", true, false},
        {"libheif", "1.19.5", "Dirk Farin",
         "https://github.com/nicedoc/libheif", LicenseType::LGPL21,
         "LGPL-2.1-only", nullptr, "pkg:github/nicedoc/libheif@1.19.5", true,
         false},
        {"libde265", "1.0.15", "Dirk Farin",
         "https://github.com/nicedoc/libde265", LicenseType::LGPL21,
         "LGPL-2.1-only", nullptr, "pkg:github/nicedoc/libde265@1.0.15", true,
         false},
        {"dav1d", "1.5.1", "VideoLAN",
         "https://code.videolan.org/videolan/dav1d", LicenseType::BSD2Clause,
         "BSD-2-Clause", nullptr, "pkg:github/nicedoc/dav1d@1.5.1", true,
         false},
        {"libraw", "0.21.3", "LibRaw LLC", "https://www.libraw.org",
         LicenseType::LGPL21, "LGPL-2.1-only", nullptr,
         "pkg:github/libraw/libraw@0.21.3", true, false},
        {"bzip2", "1.0.8", "Julian Seward", "https://sourceware.org/bzip2",
         LicenseType::BSD3Clause, "bzip2-1.0.6", nullptr, nullptr, true, false},
        {"xz-utils", "5.6.4", "Lasse Collin", "https://tukaani.org/xz",
         LicenseType::PublicDomain, "0BSD", nullptr, nullptr, true, false},
        {"highway", "1.2.0", "Google", "https://github.com/google/highway",
         LicenseType::Apache2, "Apache-2.0", nullptr,
         "pkg:github/google/highway@1.2.0", false, false},
        {"brotli", "1.1.0", "Google", "https://github.com/google/brotli",
         LicenseType::MIT, "MIT", nullptr, "pkg:github/google/brotli@1.1.0",
         false, false},
        {"lcms2", "2.16", "Marti Maria", "https://github.com/mm2/Little-CMS",
         LicenseType::MIT, "MIT", nullptr, "pkg:github/mm2/Little-CMS@2.16",
         false, false},
    };
    static const SBOMComponent empty{};
    return index < COMPONENT_COUNT ? components[index] : empty;
  }

  /// Return the display name for a given output format
  static const wchar_t *FormatName(SBOMOutputFormat f) {
    switch (f) {
    case SBOMOutputFormat::SPDX_JSON:
      return L"SPDX";
    case SBOMOutputFormat::CycloneDX:
      return L"CycloneDX";
    case SBOMOutputFormat::CSV:
      return L"CSV";
    default:
      return L"Unknown";
    }
  }

  /// Return the number of supported output formats
  static constexpr size_t FormatCount() {
    return static_cast<size_t>(SBOMOutputFormat::COUNT);
  }

  /// Generate SBOM to file
  bool GenerateSBOM(const wchar_t *outputPath,
                    SBOMOutputFormat format = SBOMOutputFormat::SPDX_JSON) {
    FILE *fp = nullptr;
    _wfopen_s(&fp, outputPath, L"w");
    if (!fp)
      return false;

    switch (format) {
    case SBOMOutputFormat::SPDX_JSON:
      WriteSPDX(fp);
      break;
    case SBOMOutputFormat::CycloneDX:
      WriteCycloneDX(fp);
      break;
    case SBOMOutputFormat::CSV:
      WriteCSV(fp);
      break;
    }

    fclose(fp);
    return true;
  }

  /// License name lookup
  static const char *LicenseName(LicenseType l) {
    switch (l) {
    case LicenseType::MIT:
      return "MIT";
    case LicenseType::BSD2Clause:
      return "BSD-2-Clause";
    case LicenseType::BSD3Clause:
      return "BSD-3-Clause";
    case LicenseType::Apache2:
      return "Apache-2.0";
    case LicenseType::LGPL21:
      return "LGPL-2.1-only";
    case LicenseType::Zlib:
      return "Zlib";
    case LicenseType::Proprietary:
      return "Proprietary";
    case LicenseType::PublicDomain:
      return "Public Domain";
    case LicenseType::ISC:
      return "ISC";
    case LicenseType::MPL2:
      return "MPL-2.0";
    case LicenseType::Custom:
      return "Custom";
    default:
      return "Unknown";
    }
  }

private:
  SBOMGenerator() = default;

  void WriteSPDX(FILE *fp) {
    fprintf(fp, "{\n");
    fprintf(fp, "  \"spdxVersion\": \"SPDX-2.3\",\n");
    fprintf(fp, "  \"dataLicense\": \"CC0-1.0\",\n");
    fprintf(fp, "  \"SPDXID\": \"SPDXRef-DOCUMENT\",\n");
    fprintf(fp, "  \"name\": \"ExplorerLens-v15.0.0\",\n");
    fprintf(
        fp,
        "  \"documentNamespace\": \"https://explorerlens.io/sbom/v15.0.0\",\n");
    fprintf(fp, "  \"packages\": [\n");
    for (uint32_t i = 0; i < COMPONENT_COUNT; ++i) {
      const auto &c = GetComponent(i);
      fprintf(fp, "    {\n");
      fprintf(fp, "      \"name\": \"%s\",\n", c.name);
      fprintf(fp, "      \"SPDXID\": \"SPDXRef-Package-%s\",\n", c.name);
      fprintf(fp, "      \"versionInfo\": \"%s\",\n", c.version);
      fprintf(fp, "      \"supplier\": \"Organization: %s\",\n", c.supplier);
      fprintf(fp, "      \"downloadLocation\": \"%s\",\n",
              c.downloadUrl ? c.downloadUrl : "NOASSERTION");
      fprintf(fp, "      \"licenseConcluded\": \"%s\"\n",
              c.licenseSpdxId ? c.licenseSpdxId : "NOASSERTION");
      fprintf(fp, "    }%s\n", (i + 1 < COMPONENT_COUNT) ? "," : "");
    }
    fprintf(fp, "  ]\n}\n");
  }

  void WriteCycloneDX(FILE *fp) {
    fprintf(fp, "{\n");
    fprintf(fp, "  \"bomFormat\": \"CycloneDX\",\n");
    fprintf(fp, "  \"specVersion\": \"1.5\",\n");
    fprintf(fp, "  \"version\": 1,\n");
    fprintf(fp, "  \"components\": [\n");
    for (uint32_t i = 0; i < COMPONENT_COUNT; ++i) {
      const auto &c = GetComponent(i);
      fprintf(fp,
              "    { \"type\": \"library\", \"name\": \"%s\", \"version\": "
              "\"%s\" }%s\n",
              c.name, c.version, (i + 1 < COMPONENT_COUNT) ? "," : "");
    }
    fprintf(fp, "  ]\n}\n");
  }

  void WriteCSV(FILE *fp) {
    fprintf(fp, "Name,Version,License,Supplier,Direct\n");
    for (uint32_t i = 0; i < COMPONENT_COUNT; ++i) {
      const auto &c = GetComponent(i);
      fprintf(fp, "%s,%s,%s,%s,%s\n", c.name, c.version, LicenseName(c.license),
              c.supplier, c.isDirectDependency ? "Yes" : "No");
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
