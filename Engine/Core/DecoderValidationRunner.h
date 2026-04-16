// DecoderValidationRunner.h — Batch corpus validation across all registered decoders
// Copyright (c) 2026 ExplorerLens Project
//
// Iterates the corpus MANIFEST.json, invokes each decoder through the
// FormatValidator, and produces a structured JSON report.
//
#pragma once
#include "FormatValidator.h"
#include "ThumbnailHashRegistry.h"
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CorpusEntry {
    std::string filePath;   // relative path under data/corpus/
    std::string formatId;   // e.g. "JPEG"
    std::string decoderId;  // e.g. "JpegDecoder"
    int         priority;   // 1=critical, 2=standard, 3=optional
};

struct RunReport {
    int       total     = 0;
    int       passed    = 0;
    int       failed    = 0;
    int       skipped   = 0;
    double    durationMs = 0.0;
    std::vector<ValidationResult> results;
};

// Decoder callback type: given a corpus file path, decode at targetSize and
// fill pixels (BGRA32).  Returns false on decode error.
using DecodeFn = std::function<bool(const std::string& filePath,
                                    uint32_t targetSize,
                                    std::vector<uint8_t>& outPixels,
                                    uint32_t& outWidth,
                                    uint32_t& outHeight,
                                    std::string& outError)>;

class DecoderValidationRunner {
public:
    explicit DecoderValidationRunner(const std::string& corpusRoot,
                                     const ThumbnailHashRegistry* registry = nullptr)
        : m_corpusRoot(corpusRoot), m_registry(registry) {}

    // Load entries from the corpus MANIFEST.json.
    // Throws std::runtime_error on I/O or parse error.
    void LoadManifest(const std::string& manifestPath);

    // Run validation for all corpus entries using the provided decoder function.
    RunReport Run(const DecodeFn&         decodeFn,
                  const ValidationConfig& cfg = {}) const;

    // Convenience: run only entries for a specific format.
    RunReport RunFormat(const std::string& formatId,
                        const DecodeFn&    decodeFn,
                        const ValidationConfig& cfg = {}) const;

    // Write a JSON report to disk.
    static void WriteReport(const RunReport& report, const std::string& outputPath);

    // Print a summary table to stdout.
    static void PrintSummary(const RunReport& report);

    const std::vector<CorpusEntry>& Entries() const noexcept { return m_entries; }

private:
    std::string                     m_corpusRoot;
    const ThumbnailHashRegistry*    m_registry;
    std::vector<CorpusEntry>        m_entries;
};

} // namespace Engine
} // namespace ExplorerLens
