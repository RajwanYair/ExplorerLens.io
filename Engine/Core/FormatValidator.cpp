// FormatValidator.cpp
// Copyright (c) 2026 ExplorerLens Project
//
#include "FormatValidator.h"
#include <sstream>

namespace ExplorerLens {
namespace Engine {

ValidationResult FormatValidator::Validate(
    const std::string&       filePath,
    const std::string&       formatId,
    std::span<const uint8_t> pixels,
    uint32_t                 width,
    uint32_t                 height,
    const ValidationConfig&  cfg) const {
    ValidationResult r;
    r.filePath = filePath;
    r.formatId = formatId;
    r.width    = width;
    r.height   = height;

    // 1. Non-empty pixel buffer
    if (pixels.empty()) {
        r.status       = ValidationStatus::FAIL_NULL_PIXELS;
        r.errorMessage = "Decoded pixel buffer is empty";
        return r;
    }

    // 2. Minimum stride check: pixels must hold at least width*height*4 bytes
    size_t expected = static_cast<size_t>(width) * height * 4;
    if (pixels.size() < expected) {
        r.status       = ValidationStatus::FAIL_WRONG_STRIDE;
        r.errorMessage = "Pixel buffer size " + std::to_string(pixels.size()) +
                         " < expected " + std::to_string(expected);
        return r;
    }

    // 3. Dimension sanity
    if (width == 0 || height == 0 || width > 32768 || height > 32768) {
        r.status       = ValidationStatus::FAIL_DIMENSIONS_OOB;
        r.errorMessage = "Dimensions out of bounds: " + std::to_string(width) +
                         "x" + std::to_string(height);
        return r;
    }

    // 4. Perceptual hash regression check
    if (m_registry) {
        auto check = m_registry->Check(filePath, pixels, width, height,
                                        cfg.dhashThreshold, cfg.phashThreshold);
        r.dhashDist = check.dhammingDist;
        r.phashDist = check.phammingDist;

        if (!check.passed) {
            if (cfg.requireHashBaseline || m_registry->Lookup(filePath).has_value()) {
                r.status       = ValidationStatus::FAIL_HASH_REGRESSION;
                r.errorMessage = check.failReason;
                return r;
            }
            // No baseline registered: treat as a first run / skip
            r.status = ValidationStatus::SKIP_NO_BASELINE;
            return r;
        }
    } else if (cfg.requireHashBaseline) {
        r.status       = ValidationStatus::SKIP_NO_BASELINE;
        r.errorMessage = "No ThumbnailHashRegistry provided";
        return r;
    }

    r.status = ValidationStatus::PASS;
    r.ssim   = 1.0f;
    return r;
}

const char* FormatValidator::StatusString(ValidationStatus s) noexcept {
    switch (s) {
        case ValidationStatus::PASS:                 return "PASS";
        case ValidationStatus::FAIL_NULL_PIXELS:     return "FAIL_NULL_PIXELS";
        case ValidationStatus::FAIL_WRONG_STRIDE:    return "FAIL_WRONG_STRIDE";
        case ValidationStatus::FAIL_DIMENSIONS_OOB:  return "FAIL_DIMENSIONS_OOB";
        case ValidationStatus::FAIL_HASH_REGRESSION: return "FAIL_HASH_REGRESSION";
        case ValidationStatus::FAIL_SSIM_REGRESSION: return "FAIL_SSIM_REGRESSION";
        case ValidationStatus::FAIL_DECODER_ERROR:   return "FAIL_DECODER_ERROR";
        case ValidationStatus::SKIP_NO_BASELINE:     return "SKIP_NO_BASELINE";
    }
    return "UNKNOWN";
}

} // namespace Engine
} // namespace ExplorerLens
