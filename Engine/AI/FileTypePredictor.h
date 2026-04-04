// FileTypePredictor.h — ML-Assisted File Type Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Uses magic byte signatures and statistical analysis to predict file
// types when extensions are missing, wrong, or ambiguous.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct FileTypeSig
{
    std::string formatName;
    std::vector<uint8_t> magicBytes;
    uint32_t offset = 0;
    float confidence = 1.0f;
};

struct PredictionResult
{
    std::string predictedType;
    float confidence = 0.0f;
    std::string extensionMatch;
    bool extensionMismatch = false;
    std::vector<std::pair<std::string, float>> alternatives;
};

struct FileTypePredictorConfig
{
    uint32_t maxBytesToRead = 4096;
    float minimumConfidence = 0.5f;
    bool trustExtensionFirst = true;
};

class FileTypePredictor
{
  public:
    void Configure(const FileTypePredictorConfig& config)
    {
        m_config = config;
    }

    void RegisterSignature(FileTypeSig sig)
    {
        m_signatures.push_back(std::move(sig));
    }

    PredictionResult Predict(const uint8_t* headerBytes, size_t headerSize, const std::string& fileExtension) const
    {
        PredictionResult result;
        float bestConf = 0.0f;

        for (const auto& sig : m_signatures) {
            if (sig.offset + sig.magicBytes.size() > headerSize)
                continue;
            bool match = true;
            for (size_t i = 0; i < sig.magicBytes.size(); ++i) {
                if (headerBytes[sig.offset + i] != sig.magicBytes[i]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                float conf = sig.confidence;
                if (conf > bestConf) {
                    bestConf = conf;
                    result.predictedType = sig.formatName;
                    result.confidence = conf;
                }
                result.alternatives.push_back({sig.formatName, conf});
            }
        }

        if (!fileExtension.empty() && m_config.trustExtensionFirst) {
            result.extensionMatch = fileExtension;
            if (!result.predictedType.empty()) {
                // Check if extension matches predicted type
                auto it = m_extensionMap.find(fileExtension);
                if (it != m_extensionMap.end() && it->second != result.predictedType) {
                    result.extensionMismatch = true;
                }
            }
        }
        return result;
    }

    void RegisterExtensionMapping(const std::string& ext, const std::string& format)
    {
        m_extensionMap[ext] = format;
    }

    size_t SignatureCount() const
    {
        return m_signatures.size();
    }

    void LoadDefaultSignatures()
    {
        RegisterSignature({"PNG", {0x89, 0x50, 0x4E, 0x47}, 0, 1.0f});
        RegisterSignature({"JPEG", {0xFF, 0xD8, 0xFF}, 0, 1.0f});
        RegisterSignature({"GIF", {0x47, 0x49, 0x46, 0x38}, 0, 1.0f});
        RegisterSignature({"BMP", {0x42, 0x4D}, 0, 0.9f});
        RegisterSignature({"WEBP", {0x57, 0x45, 0x42, 0x50}, 8, 1.0f});
        RegisterSignature({"PDF", {0x25, 0x50, 0x44, 0x46}, 0, 1.0f});
        RegisterSignature({"ZIP", {0x50, 0x4B, 0x03, 0x04}, 0, 0.95f});
        RegisterSignature({"RAR", {0x52, 0x61, 0x72, 0x21}, 0, 1.0f});
        RegisterSignature({"TIFF-LE", {0x49, 0x49, 0x2A, 0x00}, 0, 0.95f});
        RegisterSignature({"TIFF-BE", {0x4D, 0x4D, 0x00, 0x2A}, 0, 0.95f});
    }

  private:
    FileTypePredictorConfig m_config;
    std::vector<FileTypeSig> m_signatures;
    std::unordered_map<std::string, std::string> m_extensionMap;
};

}  // namespace Engine
}  // namespace ExplorerLens
