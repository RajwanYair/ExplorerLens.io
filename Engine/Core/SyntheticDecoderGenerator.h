// SyntheticDecoderGenerator.h — Synthetic Decoder Stub Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates best-effort decoder stubs for unsupported or unknown formats using
// format-family heuristics — so unknown files produce a meaningful placeholder.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecoderFamily    { Image, Document, Archive, Video, Audio, Data, Unknown };
enum class StubQuality      { Full, Partial, Placeholder, Failed };

struct SyntheticDecoderSpec {
    std::string  formatId;
    DecoderFamily family         = DecoderFamily::Unknown;
    std::string  baseDecoderHint;
    std::vector<std::string> similarFormats;
    float        anticipatedAccuracy = 0.0f;
};

struct GeneratorSynthesisResult {
    bool          success      = false;
    StubQuality   quality      = StubQuality::Failed;
    std::string   stubClassName;
    std::string   generatedCode;
    float         estimatedFidelity = 0.0f;
    std::string   errorMessage;
};

class SyntheticDecoderGenerator {
public:
    SyntheticDecoderGenerator() = default;

    GeneratorSynthesisResult Generate(const SyntheticDecoderSpec& spec) {
        GeneratorSynthesisResult r;
        if (spec.formatId.empty()) {
            r.errorMessage = "Format ID must not be empty";
            return r;
        }
        r.success          = true;
        r.stubClassName    = "Synthetic_" + spec.formatId + "_Decoder";
        r.quality          = spec.family == DecoderFamily::Unknown
                             ? StubQuality::Placeholder : StubQuality::Partial;
        r.estimatedFidelity= spec.anticipatedAccuracy > 0 ? spec.anticipatedAccuracy : 0.3f;
        r.generatedCode    = "// Auto-generated stub for " + spec.formatId + "\n";
        ++m_generatedCount;
        return r;
    }

    uint32_t      GeneratedCount()     const { return m_generatedCount; }
    DecoderFamily InferFamily(const std::string& ext) const {
        if (ext == "png" || ext == "jpg" || ext == "webp") return DecoderFamily::Image;
        if (ext == "pdf" || ext == "docx")                 return DecoderFamily::Document;
        if (ext == "zip" || ext == "rar")                  return DecoderFamily::Archive;
        return DecoderFamily::Unknown;
    }
    void  Reset() { m_generatedCount = 0; }

private:
    uint32_t m_generatedCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
