//==============================================================================
// ExplorerLens Engine - Format Detector Header
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include "../Core/IFormatDetector.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Concrete implementation of IFormatDetector
///
/// Detects file formats using:
/// 1. Extension-based detection (fast)
/// 2. File signature detection (reliable but slower)
//==============================================================================
class FormatDetector : public IFormatDetector
{
  public:
    FormatDetector();
    virtual ~FormatDetector();

    // IFormatDetector implementation
    DetectedFormat DetectFormat(const wchar_t* filePath) override;
    DetectedFormat DetectFromExtension(const wchar_t* extension) override;
    DetectedFormat DetectFromSignature(const wchar_t* filePath) override;
    bool IsImageFormat(const wchar_t* extension) const override;
    bool IsArchiveFormat(const wchar_t* extension) const override;
    bool IsDocumentFormat(const wchar_t* extension) const override;
    bool IsVideoFormat(const wchar_t* extension) const override;
    const wchar_t* GetExtension(const wchar_t* filePath) const override;

  private:
    // Non-copyable
    FormatDetector(const FormatDetector&) = delete;
    FormatDetector& operator=(const FormatDetector&) = delete;
};

}  // namespace Engine
}  // namespace ExplorerLens
