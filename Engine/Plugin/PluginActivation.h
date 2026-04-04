#pragma once
//==============================================================================
// ExplorerLens — Plugin System Activation
// Feature-flag-gated plugin loading, IPC pipeline, plugin discovery,
// lifecycle management, and sample plugin validation.
//==============================================================================

#ifndef EXPLORERLENS_PLUGIN_ACTIVATION_H
    #define EXPLORERLENS_PLUGIN_ACTIVATION_H

    #include <algorithm>
    #include <chrono>
    #include <cstdint>
    #include <functional>
    #include <memory>
    #include <sstream>
    #include <string>
    #include <unordered_map>
    #include <vector>

namespace ExplorerLens {
namespace Engine {
namespace Plugin {

//==============================================================================
// Plugin Feature Flags — gate all plugin activation behind toggles
//==============================================================================

struct PluginFeatureFlags
{
    bool enablePlugins = false;      // Master switch
    bool enableIPC = false;          // Named-pipe IPC to PluginHost.exe
    bool enableDiscovery = true;     // Auto-scan plugin directories
    bool enableMarketplace = false;  // Online plugin marketplace
    bool enableSandbox = true;       // Process-isolated plugin execution
    bool enableHotReload = false;    // Live reload on DLL change
    uint32_t maxPlugins = 32;        // Max concurrent plugins
    uint32_t ipcTimeoutMs = 5000;    // IPC call timeout

    // Standard production config
    static PluginFeatureFlags Production()
    {
        PluginFeatureFlags f;
        f.enablePlugins = true;
        f.enableIPC = true;
        f.enableDiscovery = true;
        f.enableMarketplace = false;
        f.enableSandbox = true;
        f.enableHotReload = false;
        return f;
    }

    // All features enabled (dev/test)
    static PluginFeatureFlags AllEnabled()
    {
        PluginFeatureFlags f;
        f.enablePlugins = true;
        f.enableIPC = true;
        f.enableDiscovery = true;
        f.enableMarketplace = true;
        f.enableSandbox = true;
        f.enableHotReload = true;
        return f;
    }

    // Disabled (safe mode)
    static PluginFeatureFlags Disabled()
    {
        return PluginFeatureFlags{};
    }

    size_t EnabledCount() const
    {
        size_t n = 0;
        if (enablePlugins)
            ++n;
        if (enableIPC)
            ++n;
        if (enableDiscovery)
            ++n;
        if (enableMarketplace)
            ++n;
        if (enableSandbox)
            ++n;
        if (enableHotReload)
            ++n;
        return n;
    }
};

//==============================================================================
// Plugin State Machine
//==============================================================================

enum class PluginState {
    Discovered,  // Found on disk
    Validated,   // Manifest parsed successfully
    Loading,     // DLL being loaded
    Active,      // Running and serving decode requests
    Suspended,   // Temporarily disabled (user toggle)
    Error,       // Failed to load or crashed
    Unloaded     // Explicitly unloaded
};

inline const char* PluginStateName(PluginState s)
{
    switch (s) {
        case PluginState::Discovered:
            return "Discovered";
        case PluginState::Validated:
            return "Validated";
        case PluginState::Loading:
            return "Loading";
        case PluginState::Active:
            return "Active";
        case PluginState::Suspended:
            return "Suspended";
        case PluginState::Error:
            return "Error";
        case PluginState::Unloaded:
            return "Unloaded";
    }
    return "Unknown";
}

inline bool IsOperational(PluginState s)
{
    return s == PluginState::Active || s == PluginState::Suspended;
}

//==============================================================================
// Plugin Descriptor — metadata from plugin manifest
//==============================================================================

struct PluginDescriptor
{
    std::string id;       // Unique identifier
    std::string name;     // Human-readable name
    std::string version;  // SemVer
    std::string author;
    std::string description;
    std::string dllPath;                        // Path to plugin DLL
    std::vector<std::string> supportedFormats;  // File extensions
    PluginState state = PluginState::Discovered;
    bool isSigned = false;
    uint64_t loadTimeMs = 0;
    uint64_t decodeCount = 0;
    uint64_t errorCount = 0;

    bool IsActive() const
    {
        return state == PluginState::Active;
    }
    bool HasErrors() const
    {
        return errorCount > 0;
    }

    double ErrorRate() const
    {
        if (decodeCount == 0)
            return 0.0;
        return (static_cast<double>(errorCount) / decodeCount) * 100.0;
    }

    size_t FormatCount() const
    {
        return supportedFormats.size();
    }

    std::string StatusBadge() const
    {
        std::string badge = "[";
        badge += PluginStateName(state);
        badge += "]";
        return badge;
    }
};

//==============================================================================
// Plugin Discovery — scan filesystem for plugins
//==============================================================================

class PluginDiscovery
{
  public:
    // Standard plugin search directories
    static std::vector<std::string> DefaultSearchPaths()
    {
        return {
            "%LocalAppData%\\ExplorerLens\\Plugins", "%ProgramData%\\ExplorerLens\\Plugins",
            ".\\plugins"  // Portable mode
        };
    }

    // Expected manifest filename
    static std::string ManifestFilename()
    {
        return "plugin.json";
    }

    // Scan a directory for plugins (returns descriptors)
    std::vector<PluginDescriptor> ScanDirectory(const std::string& /*path*/)
    {
        // In production, this would walk the filesystem
        // For testing, we return pre-configured results
        return scannedPlugins_;
    }

    // Register a discovered plugin (for testing)
    void AddPlugin(const PluginDescriptor& plugin)
    {
        scannedPlugins_.push_back(plugin);
    }

    size_t PluginCount() const
    {
        return scannedPlugins_.size();
    }

    void Clear()
    {
        scannedPlugins_.clear();
    }

  private:
    std::vector<PluginDescriptor> scannedPlugins_;
};

//==============================================================================
// IPC Channel — named pipe communication with PluginHost.exe
//==============================================================================

enum class IPCMessageType {
    Ping,           // Health check
    DecodeRequest,  // Send file for decoding
    DecodeResult,   // Receive decoded thumbnail
    Shutdown,       // Graceful stop
    PluginList,     // Query loaded plugins
    HealthCheck,    // Detailed health response
    Error           // Error response
};

inline const char* IPCMessageTypeName(IPCMessageType t)
{
    switch (t) {
        case IPCMessageType::Ping:
            return "Ping";
        case IPCMessageType::DecodeRequest:
            return "DecodeRequest";
        case IPCMessageType::DecodeResult:
            return "DecodeResult";
        case IPCMessageType::Shutdown:
            return "Shutdown";
        case IPCMessageType::PluginList:
            return "PluginList";
        case IPCMessageType::HealthCheck:
            return "HealthCheck";
        case IPCMessageType::Error:
            return "Error";
    }
    return "Unknown";
}

struct IPCMessage
{
    IPCMessageType type;
    std::string payload;
    uint32_t sequenceId = 0;
    bool isResponse = false;

    std::string TypeName() const
    {
        return IPCMessageTypeName(type);
    }
};

class IPCChannel
{
  public:
    explicit IPCChannel(const std::string& pipeName = "\\\\.\\pipe\\ExplorerLens-PluginHost") : pipeName_(pipeName) {}

    std::string PipeName() const
    {
        return pipeName_;
    }

    bool Connect()
    {
        connected_ = true;
        return true;
    }

    bool IsConnected() const
    {
        return connected_;
    }

    void Disconnect()
    {
        connected_ = false;
    }

    // Send message (simulated)
    bool Send(const IPCMessage& msg)
    {
        if (!connected_)
            return false;
        sentMessages_.push_back(msg);
        return true;
    }

    // Receive response (simulated)
    IPCMessage Receive()
    {
        if (!sentMessages_.empty()) {
            auto& last = sentMessages_.back();
            IPCMessage response;
            response.type =
                (last.type == IPCMessageType::Ping) ? IPCMessageType::HealthCheck : IPCMessageType::DecodeResult;
            response.isResponse = true;
            response.sequenceId = last.sequenceId;
            return response;
        }
        return {IPCMessageType::Error, "No pending messages", 0, true};
    }

    size_t MessagesSent() const
    {
        return sentMessages_.size();
    }

  private:
    std::string pipeName_;
    bool connected_ = false;
    std::vector<IPCMessage> sentMessages_;
};

//==============================================================================
// Plugin Lifecycle Manager — orchestrates plugin loading/unloading
//==============================================================================

class PluginLifecycleManager
{
  public:
    explicit PluginLifecycleManager(const PluginFeatureFlags& flags = PluginFeatureFlags::Production()) : flags_(flags)
    {}

    bool IsEnabled() const
    {
        return flags_.enablePlugins;
    }

    // Register a plugin
    bool RegisterPlugin(const PluginDescriptor& plugin)
    {
        if (!flags_.enablePlugins)
            return false;
        if (plugins_.size() >= flags_.maxPlugins)
            return false;
        plugins_[plugin.id] = plugin;
        plugins_[plugin.id].state = PluginState::Validated;
        return true;
    }

    // Activate a plugin
    bool ActivatePlugin(const std::string& id)
    {
        auto it = plugins_.find(id);
        if (it == plugins_.end())
            return false;
        it->second.state = PluginState::Active;
        return true;
    }

    // Suspend a plugin
    bool SuspendPlugin(const std::string& id)
    {
        auto it = plugins_.find(id);
        if (it == plugins_.end())
            return false;
        if (it->second.state != PluginState::Active)
            return false;
        it->second.state = PluginState::Suspended;
        return true;
    }

    // Unload a plugin
    bool UnloadPlugin(const std::string& id)
    {
        auto it = plugins_.find(id);
        if (it == plugins_.end())
            return false;
        it->second.state = PluginState::Unloaded;
        return true;
    }

    // Query
    size_t TotalPlugins() const
    {
        return plugins_.size();
    }

    size_t ActivePlugins() const
    {
        size_t n = 0;
        for (auto& [id, p] : plugins_)
            if (p.IsActive())
                ++n;
        return n;
    }

    const PluginDescriptor* GetPlugin(const std::string& id) const
    {
        auto it = plugins_.find(id);
        return (it != plugins_.end()) ? &it->second : nullptr;
    }

    std::vector<std::string> GetActivePluginIds() const
    {
        std::vector<std::string> ids;
        for (auto& [id, p] : plugins_)
            if (p.IsActive())
                ids.push_back(id);
        return ids;
    }

    // Generate status report
    std::string StatusReport() const
    {
        std::ostringstream ss;
        ss << "# Plugin Status Report\n\n";
        ss << "| Plugin | State | Formats | Decodes | Errors |\n";
        ss << "|--------|-------|---------|---------|--------|\n";
        for (auto& [id, p] : plugins_) {
            ss << "| " << p.name << " | " << PluginStateName(p.state) << " | " << p.FormatCount() << " | "
               << p.decodeCount << " | " << p.errorCount << " |\n";
        }
        return ss.str();
    }

  private:
    PluginFeatureFlags flags_;
    std::unordered_map<std::string, PluginDescriptor> plugins_;
};

//==============================================================================
// Sample Plugin Spec — minimal-plugin reference implementation
//==============================================================================

struct SamplePluginSpec
{
    static PluginDescriptor MinimalPlugin()
    {
        PluginDescriptor p;
        p.id = "com.explorerlens.minimal-plugin";
        p.name = "Minimal Plugin";
        p.version = "1.0.0";
        p.author = "ExplorerLens Team";
        p.description = "Reference implementation for ExplorerLens plugin API";
        p.dllPath = "plugins\\minimal-plugin\\minimal-plugin.dll";
        p.supportedFormats = {".custom", ".test"};
        p.isSigned = true;
        return p;
    }

    static PluginDescriptor RawEnhancedPlugin()
    {
        PluginDescriptor p;
        p.id = "com.explorerlens.raw-enhanced";
        p.name = "RAW Enhanced Decoder";
        p.version = "1.0.0";
        p.author = "ExplorerLens Team";
        p.description = "Enhanced RAW processing with custom demosaic";
        p.dllPath = "plugins\\raw-enhanced\\raw-enhanced.dll";
        p.supportedFormats = {".nef", ".cr3", ".arw", ".orf", ".rw2"};
        p.isSigned = true;
        return p;
    }
};

}  // namespace Plugin
}  // namespace Engine
}  // namespace ExplorerLens

#endif  // EXPLORERLENS_PLUGIN_ACTIVATION_H
