#pragma once
//==============================================================================
// ExplorerLens — Context Menu & Shell UX Integration
// Explorer right-click actions: Regenerate Thumbnail, Copy to Clipboard,
// Export as PNG. Shell property handler, batch folder operations.
//==============================================================================

#ifndef EXPLORERLENS_CONTEXT_MENU_HANDLER_H
#define EXPLORERLENS_CONTEXT_MENU_HANDLER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {
namespace Shell {

//==============================================================================
// Context Menu Action — individual right-click command
//==============================================================================

enum class ContextMenuAction
{
    RegenerateThumbnail, // Force re-decode and update cache
    CopyToClipboard, // Copy decoded thumbnail as PNG to clipboard
    ExportAsPNG, // Save thumbnail to user-chosen path
    RegenerateFolder, // Batch regenerate all in folder
    ShowProperties // Show format details in Explorer
};

inline const char* ContextMenuActionName(ContextMenuAction a) {
    switch (a) {
    case ContextMenuAction::RegenerateThumbnail: return "Regenerate Thumbnail";
    case ContextMenuAction::CopyToClipboard: return "Copy Thumbnail to Clipboard";
    case ContextMenuAction::ExportAsPNG: return "Export Thumbnail as PNG";
    case ContextMenuAction::RegenerateFolder: return "Regenerate All Thumbnails";
    case ContextMenuAction::ShowProperties: return "Show Format Properties";
    }
    return "Unknown";
}

// Menu item verb (used in IContextMenu::GetCommandString)
inline const char* ContextMenuVerb(ContextMenuAction a) {
    switch (a) {
    case ContextMenuAction::RegenerateThumbnail: return "explorerlens.regenerate";
    case ContextMenuAction::CopyToClipboard: return "explorerlens.copy";
    case ContextMenuAction::ExportAsPNG: return "explorerlens.export";
    case ContextMenuAction::RegenerateFolder: return "explorerlens.regenerateall";
    case ContextMenuAction::ShowProperties: return "explorerlens.properties";
    }
    return "explorerlens.unknown";
}

struct ContextMenuItem
{
    ContextMenuAction action;
    std::string displayText;
    std::string verb;
    std::string helpText;
    uint32_t iconResourceId = 0;
    bool requiresFolder = false; // Only show for folders
    bool enabled = true;

    std::string DisplayName() const { return displayText; }
    bool IsEnabled() const { return enabled; }
};

//==============================================================================
// Context Menu Handler — builds and manages context menu items
//==============================================================================

class ContextMenuHandler
{
public:
    ContextMenuHandler() {
        // Register default menu items per spec
        items_ = {
        {ContextMenuAction::RegenerateThumbnail, "Regenerate Thumbnail",
        "explorerlens.regenerate", "Force re-decode and update cached thumbnail", 0, false, true},
        {ContextMenuAction::CopyToClipboard, "Copy Thumbnail to Clipboard",
        "explorerlens.copy", "Copy decoded thumbnail as PNG to clipboard", 0, false, true},
        {ContextMenuAction::ExportAsPNG, "Export Thumbnail as PNG...",
        "explorerlens.export", "Save decoded thumbnail to chosen location", 0, false, true},
        {ContextMenuAction::RegenerateFolder, "Regenerate All Thumbnails",
        "explorerlens.regenerateall", "Re-decode all thumbnails in this folder", 0, true, true},
        };
    }

    size_t ItemCount() const { return items_.size(); }

    // Get items for file context (not folder)
    std::vector<ContextMenuItem> GetFileMenuItems() const {
        std::vector<ContextMenuItem> result;
        for (auto& item : items_)
            if (!item.requiresFolder) result.push_back(item);
        return result;
    }

    // Get items for folder context
    std::vector<ContextMenuItem> GetFolderMenuItems() const {
        std::vector<ContextMenuItem> result;
        for (auto& item : items_)
            if (item.requiresFolder) result.push_back(item);
        return result;
    }

    // Check if file extension is supported
    static bool IsSupportedExtension(const std::string& ext) {
        static const std::vector<std::string> supported = {
        ".jpg", ".jpeg", ".png", ".bmp", ".gif", ".tiff", ".tif",
        ".ico", ".webp", ".jxl", ".heif", ".heic", ".avif",
        ".psd", ".tga", ".dds", ".qoi", ".svg", ".exr", ".hdr",
        ".nef", ".cr2", ".cr3", ".arw", ".orf", ".rw2", ".dng",
        ".raf", ".srw", ".pef", ".3fr", ".dcr",
        ".zip", ".rar", ".7z", ".tar", ".cbz", ".cbr", ".cb7"
        };
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
        return std::find(supported.begin(), supported.end(), lower) != supported.end();
    }

    // Get item by action
    const ContextMenuItem* GetItem(ContextMenuAction action) const {
        for (auto& item : items_)
            if (item.action == action) return &item;
        return nullptr;
    }

private:
    std::vector<ContextMenuItem> items_;
};

//==============================================================================
// Shell Property Handler — format details for Explorer Details pane
//==============================================================================

struct ShellPropertyEntry
{
    std::string propertyName;
    std::string value;
    std::string category; // e.g., "Image", "Codec", "Performance"
};

struct FileProperties
{
    std::string filePath;
    std::string format; // e.g., "JPEG XL"
    std::string codec; // e.g., "libjxl 0.11.1"
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bitsPerPixel = 0;
    bool hasAlpha = false;
    bool isAnimated = false;
    uint32_t frameCount = 1;
    double decodeTimeMs = 0.0;
    uint64_t fileSizeBytes = 0;
    std::string colorSpace; // e.g., "sRGB", "Display P3"
    std::string iccProfile;

    std::string DimensionsString() const {
        return std::to_string(width) + " x " + std::to_string(height);
    }

    std::string FileSizeString() const {
        if (fileSizeBytes < 1024) return std::to_string(fileSizeBytes) + " B";
        if (fileSizeBytes < 1024 * 1024)
            return std::to_string(fileSizeBytes / 1024) + " KB";
        double mb = static_cast<double>(fileSizeBytes) / (1024.0 * 1024.0);
        std::ostringstream ss;
        ss.precision(1);
        ss << std::fixed << mb << " MB";
        return ss.str();
    }

    // Build property list for Shell Details pane
    std::vector<ShellPropertyEntry> ToPropertyList() const {
        std::vector<ShellPropertyEntry> props;
        props.push_back({ "Format", format, "Image" });
        props.push_back({ "Dimensions", DimensionsString(), "Image" });
        props.push_back({ "Bit Depth", std::to_string(bitsPerPixel) + " bpp", "Image" });
        props.push_back({ "Alpha Channel", hasAlpha ? "Yes" : "No", "Image" });
        props.push_back({ "Codec", codec, "Codec" });
        props.push_back({ "Decode Time", std::to_string(static_cast<int>(decodeTimeMs)) + " ms", "Performance" });
        props.push_back({ "File Size", FileSizeString(), "File" });
        if (!colorSpace.empty())
            props.push_back({ "Color Space", colorSpace, "Image" });
        if (isAnimated)
            props.push_back({ "Frames", std::to_string(frameCount), "Image" });
        return props;
    }
};

//==============================================================================
// Batch Operation — folder-wide thumbnail regeneration
//==============================================================================

enum class BatchOperationState
{
    Pending,
    Running,
    Completed,
    Cancelled,
    Error
};

inline const char* BatchStateName(BatchOperationState s) {
    switch (s) {
    case BatchOperationState::Pending: return "Pending";
    case BatchOperationState::Running: return "Running";
    case BatchOperationState::Completed: return "Completed";
    case BatchOperationState::Cancelled: return "Cancelled";
    case BatchOperationState::Error: return "Error";
    }
    return "Unknown";
}

struct BatchProgress
{
    uint32_t totalFiles = 0;
    uint32_t processed = 0;
    uint32_t succeeded = 0;
    uint32_t failed = 0;
    uint32_t skipped = 0;
    BatchOperationState state = BatchOperationState::Pending;

    double ProgressPercent() const {
        if (totalFiles == 0) return 100.0;
        return (static_cast<double>(processed) / totalFiles) * 100.0;
    }

    double SuccessRate() const {
        if (processed == 0) return 100.0;
        return (static_cast<double>(succeeded) / processed) * 100.0;
    }

    bool IsComplete() const {
        return state == BatchOperationState::Completed ||
            state == BatchOperationState::Cancelled ||
            state == BatchOperationState::Error;
    }

    std::string StatusString() const {
        std::ostringstream ss;
        ss << processed << "/" << totalFiles
            << " (" << static_cast<int>(ProgressPercent()) << "%)"
            << " - " << BatchStateName(state);
        return ss.str();
    }
};

class BatchOperationManager
{
public:
    BatchProgress StartBatch(uint32_t totalFiles) {
        current_.totalFiles = totalFiles;
        current_.processed = 0;
        current_.succeeded = 0;
        current_.failed = 0;
        current_.skipped = 0;
        current_.state = BatchOperationState::Running;
        return current_;
    }

    void RecordSuccess() {
        current_.processed++;
        current_.succeeded++;
        CheckCompletion();
    }

    void RecordFailure() {
        current_.processed++;
        current_.failed++;
        CheckCompletion();
    }

    void RecordSkip() {
        current_.processed++;
        current_.skipped++;
        CheckCompletion();
    }

    void Cancel() {
        current_.state = BatchOperationState::Cancelled;
    }

    const BatchProgress& Progress() const { return current_; }

private:
    BatchProgress current_;

    void CheckCompletion() {
        if (current_.processed >= current_.totalFiles &&
            current_.state == BatchOperationState::Running) {
            current_.state = BatchOperationState::Completed;
        }
    }
};

//==============================================================================
// Clipboard Operations — HBITMAP → PNG clipboard
//==============================================================================

struct ClipboardResult
{
    bool success = false;
    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t dataSizeBytes = 0;
    std::string format = "PNG";
    std::string errorMessage;
};

struct ExportResult
{
    bool success = false;
    std::string outputPath;
    uint64_t fileSizeBytes = 0;
    std::string format = "PNG";
    std::string errorMessage;
};

}
}
} // namespace ExplorerLens::Engine::Shell

#endif // EXPLORERLENS_CONTEXT_MENU_HANDLER_H
