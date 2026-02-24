#pragma once
#include <string>
#include <vector>
#include "../../Engine/contracts/thumbnail_contracts.h"

namespace ExplorerLens::Manager::Models {

    using namespace ExplorerLens::Engine::Contracts;

    struct PipelineStageTrace {
        std::wstring StageName;
        bool Success;
        double DurationMs;
        std::wstring Details; // "Decoded via LibAVIF", "Cache HIT memory"
    };

    struct ThumbnailTrace {
        std::wstring FilePath;
        uint64_t CorrelationId;
        ThumbnailResult Result;
        
        // List of steps taken: 
        // 1. Detection (Avif)
        // 2. Cache Lookup (Miss)
        // 3. Plugin Selection (Internal LibAvif)
        // 4. Decode (Success 12ms)
        // 5. Transform (Resize 2ms)
        // 6. Encode/Cache (Saved 1ms)
        std::vector<PipelineStageTrace> Stages;
    };

    // VerifyPreviewViewModel would use this to show a visual tree of execution
}

