// GenerateCommand.h — lens generate: Single-File & Recursive Thumbnail Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the 'lens generate' subcommand.
// Decodes the input file via ExplorerLensEngine and saves the output thumbnail.
// Supports --size, --quality, --output, --format, --format-filter, --max-size,
// --gpu-off, and --recursive options.
//
#pragma once
#include "CommandRouter.h"
#include <vector>
#include <string>

namespace ExplorerLens {
namespace CLI {

class GenerateCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"generate"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Generate thumbnail(s) for one or more files";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens generate <input> [--output <path>] [--size <px>] [--quality <0-100>]"
               L" [--max-size <px>] [--format-filter <ext,...>] [--gpu-off] [--recursive]";
    }

private:
    int GenerateSingle(const std::wstring& inputPath,
                       const std::wstring& outputPath,
                       uint32_t sizePx,
                       uint32_t quality,
                       uint32_t maxSize,
                       bool gpuOff,
                       bool verbose);

    int GenerateRecursive(const std::wstring& directory,
                          const std::wstring& outputDir,
                          uint32_t sizePx,
                          uint32_t quality,
                          uint32_t maxSize,
                          const std::vector<std::wstring>& allowedExts,
                          bool gpuOff,
                          bool verbose);
};

} // namespace CLI
} // namespace ExplorerLens
