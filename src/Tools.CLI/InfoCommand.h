// InfoCommand.h — lens info: File Format Detection & Metadata Display
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 19 (v15.4.0 "Zenith-U"): Implements the 'lens info' subcommand.
// Reports format name, detected decoder, MIME type, and file metadata.
// Supports --json output for scripting and pipeline integration.
//
#pragma once
#include "CommandRouter.h"

namespace ExplorerLens {
namespace CLI {

struct FileInfo {
    std::wstring filePath;
    std::wstring detectedFormat;
    std::wstring decoderName;
    std::wstring mimeType;
    uint64_t     fileSizeBytes = 0;
    uint32_t     widthPx       = 0;
    uint32_t     heightPx      = 0;
    uint32_t     bitDepth      = 0;
    std::wstring colorSpace;
    bool         hasAlpha      = false;
    bool         isAnimated    = false;
    std::wstring extraMetadata;
};

class InfoCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"info"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Show format detection and metadata for a file";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens info <file> [--json]";
    }

private:
    FileInfo DetectFile(const std::wstring& path);
    void PrintText(const FileInfo& info) const;
    void PrintJson(const FileInfo& info) const;
};

} // namespace CLI
} // namespace ExplorerLens
