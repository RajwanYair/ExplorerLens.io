#pragma once

#include <cstdint>
#include <vector>

namespace ExplorerLens::Engine::Cpu {

    enum class CpuInstructionSet {
        Generic,
        SSE4_1,
        AVX2,
        AVX512
    };

    struct ImageBuffer {
        uint8_t* data;
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        uint32_t format; 
    };

    class ISimdDispatcher {
    public:
        virtual ~ISimdDispatcher() = default;

        // Initialize and detect CPU features
        virtual void Initialize() = 0;

        // Get detected support level
        virtual CpuInstructionSet GetSupportedInstructionSet() const = 0;

        // --- Optimized Kernels ---
        
        // Resize image using Lanczos3 or similar high-quality filter
        virtual void Resize(const ImageBuffer& src, ImageBuffer& dst) = 0;

        // Convert pixel formats (e.g. RGBA -> BGRA, or F16 -> U8)
        virtual void ConvertFormat(const ImageBuffer& src, ImageBuffer& dst) = 0;

        // Premultiply alpha
        virtual void PremultiplyAlpha(ImageBuffer& buffer) = 0;
    };

    // Factory that returns the best implementation for the running CPU
    ISimdDispatcher* GetSimdDispatcher();

}

