#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace DarkThumbs::Engine::Gpu {

    struct ShaderKey {
        uint32_t pipelineVersion;
        uint32_t shaderType; // Vertex, Pixel, Compute
        uint64_t sourceHash;
        uint64_t specializationConstantHash; // For static permutations
    };

    class IShaderCache {
    public:
        virtual ~IShaderCache() = default;

        // Try to retrieve a compiled blob from memory or disk
        virtual bool TryGetShader(const ShaderKey& key, std::vector<uint8_t>& outBytecode) = 0;

        // Store a newly compiled blob
        virtual void StoreShader(const ShaderKey& key, const std::vector<uint8_t>& bytecode) = 0;

        // Persist in-memory cache to disk (e.g. on shutdown)
        virtual void FlushToDisk() = 0;

        // Clear cache (e.g. on version upgrade or forcing rebuild)
        virtual void Invalidate() = 0;
    };

}
