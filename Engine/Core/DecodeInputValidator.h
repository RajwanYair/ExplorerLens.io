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
    uint32_t maxWidthPx      = 32768u;
    uint32_t maxHeightPx     = 32768u;
    uint64_t maxFileSizeBytes = 512ull * 1024 * 1024;  // 512 MB
    uint32_t maxBitDepth     = 32u;
};

enum class InputValidationResult
{
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

    static InputValidationResult ValidateFileSize(
        uint64_t fileSizeBytes,
        const DecodeInputConstraints& constraints = DefaultConstraints()) noexcept
    {
        if (fileSizeBytes == 0) return InputValidationResult::UnreadableHeader;
        if (fileSizeBytes > constraints.maxFileSizeBytes) return InputValidationResult::FileTooLarge;
        return InputValidationResult::Ok;
    }

    static InputValidationResult ValidateDimensions(
        uint32_t width,
        uint32_t height,
        const DecodeInputConstraints& constraints = DefaultConstraints()) noexcept
    {
        if (width == 0 || height == 0) return InputValidationResult::UnreadableHeader;
        if (width > constraints.maxWidthPx || height > constraints.maxHeightPx)
            return InputValidationResult::DimensionsTooLarge;
        return InputValidationResult::Ok;
    }

    static InputValidationResult ValidateBitDepth(
        uint32_t bitDepth,
        const DecodeInputConstraints& constraints = DefaultConstraints()) noexcept
    {
        if (bitDepth == 0 || bitDepth > constraints.maxBitDepth)
            return InputValidationResult::BitDepthExceeded;
        return InputValidationResult::Ok;
    }

    static std::string_view ResultString(InputValidationResult r) noexcept
    {
        switch (r)
        {
            case InputValidationResult::Ok:                 return "Ok";
            case InputValidationResult::FileTooLarge:       return "FileTooLarge";
            case InputValidationResult::DimensionsTooLarge: return "DimensionsTooLarge";
            case InputValidationResult::BitDepthExceeded:   return "BitDepthExceeded";
            case InputValidationResult::NullStream:         return "NullStream";
            case InputValidationResult::UnreadableHeader:   return "UnreadableHeader";
            default:                                        return "Unknown";
        }
    }
};

} // namespace Engine
} // namespace ExplorerLens
