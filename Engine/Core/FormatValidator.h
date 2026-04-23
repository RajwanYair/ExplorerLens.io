// FormatValidator.h — Per-corpus-file decoder output validation
// Copyright (c) 2026 ExplorerLens Project
//
// Validates a single decoded thumbnail against criteria:
//   - Pixel buffer is non-empty and has the expected stride
//   - Dimensions are within sane bounds
//   - Perceptual hash matches the stored baseline (ThumbnailHashRegistry)
//   - SSIM comparison with a stored reference image (optional)
//
#pragma once
#include "PerceptualHashUtility.h"
#include "SSIMComparator.h"
#include "ThumbnailHashRegistry.h"
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ValidationStatus {
    PASS,
    FAIL_NULL_PIXELS,
    FAIL_WRONG_STRIDE,
    FAIL_DIMENSIONS_OOB,
    FAIL_HASH_REGRESSION,
    FAIL_SSIM_REGRESSION,
    FAIL_DECODER_ERROR,
    SKIP_NO_BASELINE,
};

struct ValidationResult {
    ValidationStatus status          = ValidationStatus::PASS;
    std::string      filePath;
    std::string      formatId;
    uint32_t         width           = 0;
    uint32_t         height          = 0;
    float            ssim            = 1.0f;
    int              dhashDist       = 0;
    int              phashDist       = 0;
    std::string      errorMessage;

    bool Passed() const noexcept { return status == ValidationStatus::PASS ||
                                          status == ValidationStatus::SKIP_NO_BASELINE; }
};

struct ValidationConfig {
    uint32_t targetSize         = 256;
    int      dhashThreshold     = 8;
    int      phashThreshold     = 10;
    float    ssimThreshold      = SSIM_PASS_THRESHOLD;
    bool     requireHashBaseline = false;   // if true, SKIP_NO_BASELINE → FAIL
};

class FormatValidator {
public:
    explicit FormatValidator(const ThumbnailHashRegistry* registry = nullptr)
        : m_registry(registry) {}

    // Validate a decoded thumbnail against the corpus baseline.
    // The registry pointer may be null — in which case hash checks are skipped.
    ValidationResult Validate(const std::string&       filePath,
                              const std::string&       formatId,
                              std::span<const uint8_t> pixels,
                              uint32_t                 width,
                              uint32_t                 height,
                              const ValidationConfig&  cfg = {}) const;

    // Map status to human-readable string
    static const char* StatusString(ValidationStatus s) noexcept;

private:
    const ThumbnailHashRegistry* m_registry;
};

} // namespace Engine
} // namespace ExplorerLens
