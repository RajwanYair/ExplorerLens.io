// ExportCommand.h — lens export: Save a thumbnail to a file
// Copyright (c) 2026 ExplorerLens Project
//
// Implements 'lens export <source-file> <output.png> [options]'
// which decodes and saves a thumbnail to disk in PNG, BMP, or JPEG format.
//
// Usage:
//   lens export <input> <output> [--size <px>] [--format <png|bmp|jpg>]
//               [--quality <1-100>] [--overwrite] [--verbose]
//
#pragma once
#include "CommandRouter.h"

namespace ExplorerLens {
namespace CLI {

class ExportCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"export"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Decode and export a thumbnail to an image file";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens export <input-file> <output-file>\n"
               L"           [--size <px>] [--format <png|bmp|jpg>]\n"
               L"           [--quality <1-100>] [--overwrite] [--verbose]";
    }

private:
    // Write BGRA32 pixels to output path using GDI+ or WIC.
    // Returns 0 on success, non-zero with error message on failure.
    static int WriteImage(const std::wstring&      outputPath,
                          const std::vector<uint8_t>& pixels,
                          uint32_t                 width,
                          uint32_t                 height,
                          const std::wstring&      format,
                          int                      jpegQuality);
};

} // namespace CLI
} // namespace ExplorerLens
