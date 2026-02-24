#pragma once

namespace ExplorerLens::Engine::AI {

    enum class AIProcessingLevel {
        None = 0,               // Strictly disabled
        LocalOnly = 1,          // Inference on device only (DirectML/CPU)
        Hybrid = 2              // Allow cloud offload (Future)
    };

    struct AIPolicy {
        bool enabled;
        AIProcessingLevel processingLevel;
        bool allowNSFWScanning; // If false, skip safety model
        bool allowFaceDetection; // Privacy control
        bool requireSignedModels; // Security control
        
        // Factory defaults (Conservative)
        static AIPolicy Default() {
            return {
                true,
                AIProcessingLevel::LocalOnly,
                false,
                true,
                true
            };
        }
        
        // Strict Enterprise defaults
        static AIPolicy Strict() {
            return {
                false,
                AIProcessingLevel::None,
                false,
                false,
                true
            };
        }
    };

}

