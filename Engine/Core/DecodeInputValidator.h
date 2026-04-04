// DecodeInputValidator.h — Per-Decoder Input Validation Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 9 (v15.3.0 "Zenith-T"): Centralises maximum dimension, file-size, and
// colour-depth checks that must be enforced before every decoder runs.
// All decoders must call DecodeInputValidator::Validate() prior to decode operations.
//
#pragma once

#include <cstdint>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

struct DecodeInputConstraints
{
    uint32_t maxWidthPx = 32768u;
    uint32_t maxHeightPx = 32768u;
    uint64_t maxFileSizeBytes = 512ull * 1024 * 1024;  // 512 MB
    uint32_t maxBitDepth = 32u;
};

enum class DecodeValidationResult {
    Ok,
    FileTooLarge,
    DimensionsTooLarge,
    BitDepthExceeded,
    NullStream,
    UnreadableHeader
};

class DecodeInputValidator
{
  public:
    static constexpr DecodeInputConstraints DefaultConstraints() noexcept
    {
        return {};
    }

    static DecodeValidationResult ValidateFileSize(
        uint64_t fileSizeBytes, const DecodeInputConstraints& constraints = DefaultConstraints()) noexcept
    {
        if (fileSizeBytes == 0)
            return DecodeValidationResult::UnreadableHeader;
        if (fileSizeBytes > constraints.maxFileSizeBytes)
            return DecodeValidationResult::FileTooLarge;
        return DecodeValidationResult::Ok;
    }

    static DecodeValidationResult ValidateDimensions(
        uint32_t width, uint32_t height, const DecodeInputConstraints& constraints = DefaultConstraints()) noexcept
    {
        if (width == 0 || height == 0)
            return DecodeValidationResult::UnreadableHeader;
        if (width > constraints.maxWidthPx || height > constraints.maxHeightPx)
            return DecodeValidationResult::DimensionsTooLarge;
        return DecodeValidationResult::Ok;
    }

    static DecodeValidationResult ValidateBitDepth(
        uint32_t bitDepth, const DecodeInputConstraints& constraints = DefaultConstraints()) noexcept
    {
        if (bitDepth == 0 || bitDepth > constraints.maxBitDepth)
            return DecodeValidationResult::BitDepthExceeded;
        return DecodeValidationResult::Ok;
    }

    static std::string_view ResultString(DecodeValidationResult r) noexcept
    {
        switch (r) {
            case DecodeValidationResult::Ok:
                return "Ok";
            case DecodeValidationResult::FileTooLarge:
                return "FileTooLarge";
            case DecodeValidationResult::DimensionsTooLarge:
                return "DimensionsTooLarge";
            case DecodeValidationResult::BitDepthExceeded:
                return "BitDepthExceeded";
            case DecodeValidationResult::NullStream:
                return "NullStream";
            case DecodeValidationResult::UnreadableHeader:
                return "UnreadableHeader";
            default:
                return "Unknown";
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
