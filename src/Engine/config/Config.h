#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs::Engine::Config {

    struct ConfigValue {
        enum Source {
            Default,
            User,
            Machine,
            Policy
        } source;
        // Simplified value hold for now
    };

    struct EngineConfig {
        bool enableGpu;
        bool enableNetwork;
        bool allowScriptPlugins;
        uint32_t maxCacheSizeMB;
        uint32_t maxMemoryUsageMB;
        std::vector<std::string> disabledDecoders;
        
        // Metadata about where the setting came from (for diagnostics)
        ConfigValue::Source enableGpuSource;
        // ... etc
    };

    class IConfigurationProvider {
    public:
        virtual ~IConfigurationProvider() = default;
        
        // Returns the merged configuration applying Policy > Machine > User > Defaults
        virtual EngineConfig GetEffectiveConfig() const = 0;
        
        // Refreshes config from registry/disk
        virtual void Reload() = 0;
    };

}
