//==============================================================================
// ExplorerLens Engine - Format Detector Interface
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include "../Types.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Interface for file format detection
///
/// Detects file formats based on extensions and file signatures (magic bytes).
/// Used by the pipeline to route files to appropriate decoders.
//==============================================================================
class IFormatDetector {
public:
    virtual ~IFormatDetector() = default;

    //==========================================================================
    /// Detect the format of a file
    ///
    /// @param filePath Full path to the file
    /// @return Detected format type
    ///
    /// @note First attempts extension-based detection, then falls back to
    /// file signature detection if needed
    //==========================================================================
    virtual DetectedFormat DetectFormat(const wchar_t* filePath) = 0;

    //==========================================================================
    /// Detect format by extension only
    ///
    /// @param extension File extension (including the dot, e.g., ".jpg")
    /// @return Detected format type
    //==========================================================================
    virtual DetectedFormat DetectFromExtension(const wchar_t* extension) = 0;

    //==========================================================================
    /// Detect format by reading file signature (magic bytes)
    ///
    /// @param filePath Full path to the file
    /// @return Detected format type
    ///
    /// @note Opens the file and reads first 16 bytes to check signature
    //==========================================================================
    virtual DetectedFormat DetectFromSignature(const wchar_t* filePath) = 0;

    //==========================================================================
    /// Check if an extension represents an image format
    ///
    /// @param extension File extension (e.g., ".jpg", ".png")
    /// @return true if the extension is for an image format
    //==========================================================================
    virtual bool IsImageFormat(const wchar_t* extension) const = 0;

    //==========================================================================
    /// Check if an extension represents an archive format
    ///
    /// @param extension File extension (e.g., ".zip", ".cbz", ".rar")
    /// @return true if the extension is for an archive format
    //==========================================================================
    virtual bool IsArchiveFormat(const wchar_t* extension) const = 0;

    //==========================================================================
    /// Check if an extension represents a document format
    ///
    /// @param extension File extension (e.g., ".pdf", ".epub")
    /// @return true if the extension is for a document format
    //==========================================================================
    virtual bool IsDocumentFormat(const wchar_t* extension) const = 0;

    //==========================================================================
    /// Check if an extension represents a video format
    ///
    /// @param extension File extension (e.g., ".mp4", ".mkv")
    /// @return true if the extension is for a video format
    //==========================================================================
    virtual bool IsVideoFormat(const wchar_t* extension) const = 0;

    //==========================================================================
    /// Get the extension from a file path
    ///
    /// @param filePath Full file path
    /// @return Pointer to the extension within filePath (including dot)
    /// Returns nullptr if no extension found
    //==========================================================================
    virtual const wchar_t* GetExtension(const wchar_t* filePath) const = 0;
};

} // namespace Engine
} // namespace ExplorerLens
