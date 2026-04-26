// Engine/Core/StbImageAuditContract.h
// Sprint S302 — stb_image removal audit (ROADMAP v7.0 §19 / ADR A— / H28)
//
// PURPOSE:
//   Tracks every stb_image usage site scheduled for replacement in Phase 2.
//   This header is a compile-time audit marker only — it has zero runtime cost.
//   When stb_image is fully removed, delete this file and remove it from ENGINE_HEADERS.
//
// STATUS:  TODO (Phase 2 — S318/S319 will remove stb_image JPEG/PNG paths)
//
// AUDIT TABLE (as of v39.2.0):
//   File                                          | Usage               | Replacement
//   Engine/Core/EncoderExportEngine.cpp           | stb_image_write     | libjpeg-turbo / libspng
//   Engine/Decoders/ExampleDecoder.cpp            | stb_image_resize    | SIMD resize kernel
//   Engine/Tests/Catch2Tests/DecoderPriorityTier* | HDR / TGA entries   | Radiance RGBE / minimal TGA
//   Engine/Decoders/LibPngDecoderStub.h           | comment reference   | libspng (S319)
//
// SEARCH COMMAND (run before closing this out):
//   grep -r "stb_image" Engine/ --include="*.h" --include="*.cpp"
//   Expected result after S318/S319: zero matches outside this file.

#pragma once

namespace ExplorerLens::Engine {

/// Compile-time sentinel — presence of this struct means stb_image has not
/// yet been fully removed.  Delete this struct (and file) after S319.
struct StbImageAuditPending {
    /// Number of call sites still referencing stb_image APIs.
    /// Update this count after each TODO(S302) site is resolved.
    static constexpr int kRemainingUsageSites = 4;

    /// Phase 2 milestone sprint that removes the last stb_image reference.
    static constexpr int kTargetSprintRemoval = 319;

    /// Replacement libraries
    static constexpr const char* kJpegReplacement = "libjpeg-turbo 3.0";
    static constexpr const char* kPngReplacement  = "libspng 0.7";
    static constexpr const char* kHdrReplacement  = "inline Radiance RGBE parser";
    static constexpr const char* kTgaReplacement  = "inline minimal TGA reader";
};

} // namespace ExplorerLens::Engine
