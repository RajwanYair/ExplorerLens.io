// DecoderValidationRunner.cpp
// Copyright (c) 2026 ExplorerLens Project
//
#include "DecoderValidationRunner.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Minimal JSON object-array parser (reuses the string extraction pattern)
// ---------------------------------------------------------------------------

namespace {

std::string JsonStr(const std::string& obj, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t pos = obj.find(needle);
    if (pos == std::string::npos) return {};
    pos = obj.find('"', pos + needle.size());
    if (pos == std::string::npos) return {};
    ++pos;
    size_t end = obj.find('"', pos);
    return (end != std::string::npos) ? obj.substr(pos, end - pos) : std::string{};
}

int JsonInt(const std::string& obj, const std::string& key, int def = 0) {
    std::string needle = "\"" + key + "\":";
    size_t pos = obj.find(needle);
    if (pos == std::string::npos) return def;
    size_t nPos = obj.find_first_of("0123456789", pos + needle.size());
    if (nPos == std::string::npos) return def;
    return std::stoi(obj.substr(nPos));
}

std::vector<std::string> SplitObjects(const std::string& s) {
    std::vector<std::string> out;
    int depth = 0;
    size_t start = std::string::npos;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if      (c == '{') { if (depth++ == 0) start = i; }
        else if (c == '}') { if (--depth == 0 && start != std::string::npos) {
            out.push_back(s.substr(start, i - start + 1));
            start = std::string::npos;
        }}
    }
    return out;
}

// Extract all category arrays
std::vector<CorpusEntry> ParseManifest(const std::string& json) {
    std::vector<CorpusEntry> entries;

    // We need to find "categories" → each category → "formats" array
    const std::string catKey = "\"categories\"";
    size_t catPos = json.find(catKey);
    if (catPos == std::string::npos) return entries;

    // Find the opening brace of categories object
    size_t catStart = json.find('{', catPos);
    if (catStart == std::string::npos) return entries;

    // Collect nested objects — simplified: scan for "formats" arrays
    size_t searchFrom = catStart;
    while (searchFrom < json.size()) {
        size_t fmtKey = json.find("\"formats\"", searchFrom);
        if (fmtKey == std::string::npos) break;

        // Find the enclosing category to get its path
        size_t arrStart = json.find('[', fmtKey);
        if (arrStart == std::string::npos) break;
        size_t arrEnd = json.find(']', arrStart);
        if (arrEnd == std::string::npos) break;

        // Find parent category path (look back for "path": "X/")
        std::string before = json.substr(catStart, fmtKey - catStart);
        size_t pathPos = before.rfind("\"path\"");
        std::string pathPrefix;
        if (pathPos != std::string::npos) {
            size_t pq = before.find('"', pathPos + 7);
            if (pq != std::string::npos) {
                ++pq;
                size_t pe = before.find('"', pq);
                if (pe != std::string::npos) pathPrefix = before.substr(pq, pe - pq);
            }
        }

        std::string arrayContent = json.substr(arrStart + 1, arrEnd - arrStart - 1);
        for (const auto& obj : SplitObjects(arrayContent)) {
            CorpusEntry e;
            std::string testFile = JsonStr(obj, "test_file");
            if (testFile.empty()) { searchFrom = arrEnd + 1; continue; }
            e.filePath  = pathPrefix + testFile;
            e.formatId  = JsonStr(obj, "name");
            e.decoderId = JsonStr(obj, "decoder");
            e.priority  = JsonInt(obj, "priority", 2);
            entries.push_back(std::move(e));
        }
        searchFrom = arrEnd + 1;
    }
    return entries;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// LoadManifest
// ---------------------------------------------------------------------------

void DecoderValidationRunner::LoadManifest(const std::string& manifestPath) {
    std::ifstream f(manifestPath);
    if (!f.is_open())
        throw std::runtime_error("DecoderValidationRunner: cannot open " + manifestPath);
    std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    m_entries = ParseManifest(json);
}

// ---------------------------------------------------------------------------
// Run
// ---------------------------------------------------------------------------

RunReport DecoderValidationRunner::Run(const DecodeFn& decodeFn,
                                        const ValidationConfig& cfg) const {
    FormatValidator validator(m_registry);
    RunReport report;

    auto t0 = std::chrono::steady_clock::now();

    for (const auto& entry : m_entries) {
        std::string fullPath = m_corpusRoot + "/" + entry.filePath;
        std::vector<uint8_t> pixels;
        uint32_t w = 0, h = 0;
        std::string err;

        ++report.total;

        bool ok = decodeFn(fullPath, cfg.targetSize, pixels, w, h, err);
        if (!ok) {
            ValidationResult r;
            r.filePath     = entry.filePath;
            r.formatId     = entry.formatId;
            r.status       = ValidationStatus::FAIL_DECODER_ERROR;
            r.errorMessage = err;
            report.results.push_back(r);
            ++report.failed;
            continue;
        }

        auto result = validator.Validate(entry.filePath, entry.formatId,
                                          pixels, w, h, cfg);
        report.results.push_back(result);
        if (result.Passed()) ++report.passed;
        else if (result.status == ValidationStatus::SKIP_NO_BASELINE) ++report.skipped;
        else ++report.failed;
    }

    auto t1 = std::chrono::steady_clock::now();
    report.durationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return report;
}

// ---------------------------------------------------------------------------
// RunFormat
// ---------------------------------------------------------------------------

RunReport DecoderValidationRunner::RunFormat(const std::string& fmtId,
                                              const DecodeFn& decodeFn,
                                              const ValidationConfig& cfg) const {
    FormatValidator validator(m_registry);
    RunReport report;
    auto t0 = std::chrono::steady_clock::now();

    for (const auto& entry : m_entries) {
        if (entry.formatId != fmtId && entry.decoderId != fmtId) continue;
        std::string fullPath = m_corpusRoot + "/" + entry.filePath;
        std::vector<uint8_t> pixels;
        uint32_t w = 0, h = 0;
        std::string err;
        ++report.total;

        bool ok = decodeFn(fullPath, cfg.targetSize, pixels, w, h, err);
        if (!ok) {
            ValidationResult r;
            r.filePath = entry.filePath; r.formatId = entry.formatId;
            r.status = ValidationStatus::FAIL_DECODER_ERROR; r.errorMessage = err;
            report.results.push_back(r); ++report.failed; continue;
        }
        auto result = validator.Validate(entry.filePath, entry.formatId, pixels, w, h, cfg);
        report.results.push_back(result);
        if (result.Passed()) ++report.passed;
        else if (result.status == ValidationStatus::SKIP_NO_BASELINE) ++report.skipped;
        else ++report.failed;
    }

    auto t1 = std::chrono::steady_clock::now();
    report.durationMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return report;
}

// ---------------------------------------------------------------------------
// WriteReport
// ---------------------------------------------------------------------------

void DecoderValidationRunner::WriteReport(const RunReport& report,
                                           const std::string& outputPath) {
    std::ofstream f(outputPath);
    if (!f.is_open())
        throw std::runtime_error("DecoderValidationRunner: cannot write report to " + outputPath);

    f << "{\n"
      << "  \"total\": "    << report.total    << ",\n"
      << "  \"passed\": "   << report.passed   << ",\n"
      << "  \"failed\": "   << report.failed   << ",\n"
      << "  \"skipped\": "  << report.skipped  << ",\n"
      << "  \"duration_ms\": " << report.durationMs << ",\n"
      << "  \"results\": [\n";

    for (size_t i = 0; i < report.results.size(); ++i) {
        const auto& r = report.results[i];
        f << "    {\n"
          << "      \"file\": \""     << r.filePath                                 << "\",\n"
          << "      \"format\": \""   << r.formatId                                 << "\",\n"
          << "      \"status\": \""   << FormatValidator::StatusString(r.status)    << "\",\n"
          << "      \"passed\": "     << (r.Passed() ? "true" : "false")            << ",\n"
          << "      \"dhash_dist\": " << r.dhashDist                                << ",\n"
          << "      \"phash_dist\": " << r.phashDist                                << ",\n"
          << "      \"ssim\": "       << r.ssim                                     << ",\n"
          << "      \"error\": \""    << r.errorMessage                             << "\"\n"
          << "    }" << (i + 1 < report.results.size() ? "," : "") << "\n";
    }
    f << "  ]\n}\n";
}

// ---------------------------------------------------------------------------
// PrintSummary
// ---------------------------------------------------------------------------

void DecoderValidationRunner::PrintSummary(const RunReport& report) {
    std::cout << "\n=== Corpus Validation Summary ===\n"
              << "  Total   : " << report.total    << "\n"
              << "  Passed  : " << report.passed   << "\n"
              << "  Failed  : " << report.failed   << "\n"
              << "  Skipped : " << report.skipped  << "\n"
              << "  Duration: " << report.durationMs << " ms\n";
    if (report.failed > 0) {
        std::cout << "\nFailed files:\n";
        for (const auto& r : report.results) {
            if (!r.Passed() && r.status != ValidationStatus::SKIP_NO_BASELINE)
                std::cout << "  [" << FormatValidator::StatusString(r.status) << "] "
                          << r.filePath << " — " << r.errorMessage << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace Engine
} // namespace ExplorerLens
