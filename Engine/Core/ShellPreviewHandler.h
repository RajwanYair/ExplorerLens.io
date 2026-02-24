#pragma once
// Shell Preview Handler — IPreviewHandler COM for Windows file preview pane
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Preview rendering mode
enum class PreviewMode : uint32_t {
    Thumbnail  = 0,   ///< Static thumbnail
    FullImage  = 1,   ///< Full-resolution image
    Filmstrip  = 2,   ///< Multi-page filmstrip
    Document   = 3,   ///< Document text preview
    HexDump    = 4,   ///< Binary hex view
    COUNT      = 5
};

/// Preview state
enum class PreviewState : uint32_t {
    Unloaded   = 0,
    Loading    = 1,
    Ready      = 2,
    Error      = 3,
    COUNT      = 4
};

/// Preview rendering parameters
struct PreviewParams {
    std::wstring filePath;
    uint32_t     width  = 800;
    uint32_t     height = 600;
    PreviewMode  mode   = PreviewMode::FullImage;
    bool         useGPU = true;
    double       zoom   = 1.0;
};

/// Manages file preview rendering for the Windows Preview Pane
class ShellPreviewHandler {
public:
    ShellPreviewHandler();

    static const wchar_t* GetModeName(PreviewMode mode);
    static const wchar_t* GetStateName(PreviewState state);
    static uint32_t GetModeCount() { return static_cast<uint32_t>(PreviewMode::COUNT); }

    /// Load a file for preview
    bool LoadFile(const PreviewParams& params);
    /// Get current state
    PreviewState GetState() const { return m_state; }
    /// Get current params
    const PreviewParams& GetParams() const { return m_params; }
    /// Detect best preview mode for a file extension
    static PreviewMode DetectMode(const std::wstring& extension);
    /// Unload current preview
    void Unload();

private:
    PreviewState  m_state = PreviewState::Unloaded;
    PreviewParams m_params;
};

}} // namespace ExplorerLens::Engine

