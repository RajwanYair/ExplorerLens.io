// DecodeErrorCategory.h — Structured Decode Error Classification
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 10 (v15.3.0 "Zenith-T"): Extends DecodeErrorCategory with a full
// error taxonomy for the decode pipeline. The canonical enum definition lives
// in Expected.h; this header provides utility helpers built on top of it.
//
#pragma once

#include <string_view>
#include "Expected.h"

namespace ExplorerLens {
namespace Engine {

inline std::string_view DecodeErrorCategoryString(DecodeErrorCategory cat) noexcept
{
    switch (cat) {
        case DecodeErrorCategory::None:
            return "None";
        case DecodeErrorCategory::InvalidFormat:
            return "InvalidFormat";
        case DecodeErrorCategory::UnsupportedVersion:
            return "UnsupportedVersion";
        case DecodeErrorCategory::CorruptedData:
            return "CorruptedData";
        case DecodeErrorCategory::TruncatedStream:
            return "TruncatedStream";
        case DecodeErrorCategory::MissingRequiredChunk:
            return "MissingRequiredChunk";
        case DecodeErrorCategory::OutOfMemory:
            return "OutOfMemory";
        case DecodeErrorCategory::FileTooLarge:
            return "FileTooLarge";
        case DecodeErrorCategory::DimensionsTooLarge:
            return "DimensionsTooLarge";
        case DecodeErrorCategory::Timeout:
            return "Timeout";
        case DecodeErrorCategory::ZipBombDetected:
            return "ZipBombDetected";
        case DecodeErrorCategory::PathTraversalDetected:
            return "PathTraversalDetected";
        case DecodeErrorCategory::SymlinkAttackDetected:
            return "SymlinkAttackDetected";
        case DecodeErrorCategory::EncryptedAndLocked:
            return "EncryptedAndLocked";
        case DecodeErrorCategory::PolicyViolation:
            return "PolicyViolation";
        case DecodeErrorCategory::DecoderInitFailed:
            return "DecoderInitFailed";
        case DecodeErrorCategory::DecoderCrashed:
            return "DecoderCrashed";
        case DecodeErrorCategory::DecoderUnsupported:
            return "DecoderUnsupported";
        case DecodeErrorCategory::LibraryError:
            return "LibraryError";
        case DecodeErrorCategory::FileNotFound:
            return "FileNotFound";
        case DecodeErrorCategory::PermissionDenied:
            return "PermissionDenied";
        case DecodeErrorCategory::IOError:
            return "IOError";
        case DecodeErrorCategory::GPUResourceUnavailable:
            return "GPUResourceUnavailable";
        default:
            return "Unknown";
    }
}

inline bool IsSecurityError(DecodeErrorCategory cat) noexcept
{
    return cat == DecodeErrorCategory::ZipBombDetected || cat == DecodeErrorCategory::PathTraversalDetected
           || cat == DecodeErrorCategory::SymlinkAttackDetected || cat == DecodeErrorCategory::PolicyViolation;
}

inline bool IsRecoverable(DecodeErrorCategory cat) noexcept
{
    switch (cat) {
        case DecodeErrorCategory::None:
        case DecodeErrorCategory::InvalidFormat:
        case DecodeErrorCategory::UnsupportedVersion:
        case DecodeErrorCategory::Timeout:
        case DecodeErrorCategory::DecoderUnsupported:
        case DecodeErrorCategory::FileNotFound:
        case DecodeErrorCategory::GPUResourceUnavailable:
            return true;
        default:
            return false;
    }
}

}  // namespace Engine
}  // namespace ExplorerLens
