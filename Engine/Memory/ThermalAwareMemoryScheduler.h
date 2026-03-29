// ThermalAwareMemoryScheduler.h — Thermal-Aware Memory Allocation Scheduler
// Copyright (c) 2026 ExplorerLens Project
//
// Adjusts memory pre-allocation and cache warm-up schedules based on real-time
// thermal sensor readings to prevent thermal runaway during bulk thumbnail generation.
//
#pragma once
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class TAMZone          { Comfortable, Warm, Hot, Critical };
enum class MemoryScheduleAction { FullPrealloc, PartialPrealloc, LazyAlloc, Emergency };

struct ThermalMemoryConfig {
    float comfortableThresholdC = 65.0f;
    float warmThresholdC        = 78.0f;
    float hotThresholdC         = 88.0f;
    float criticalThresholdC    = 95.0f;
    int   fullPreallocMB        = 256;
    int   partialPreallocMB     = 128;
    int   lazyAllocMB           = 64;
};

struct ThermalScheduleDecision {
    TAMZone              zone       = TAMZone::Comfortable;
    MemoryScheduleAction action     = MemoryScheduleAction::FullPrealloc;
    int                  allocMB    = 256;
    bool                 delayWarmup = false;
};

class ThermalAwareMemoryScheduler {
public:
    explicit ThermalAwareMemoryScheduler(ThermalMemoryConfig config = {})
        : m_config(std::move(config)) {}

    ThermalScheduleDecision Evaluate(float thermalC) const noexcept {
        TAMZone zone;
        MemoryScheduleAction action;
        int allocMB;
        bool delay = false;

        if      (thermalC >= m_config.criticalThresholdC) { zone = TAMZone::Critical;    action = MemoryScheduleAction::Emergency;      allocMB = 16;  delay = true; }
        else if (thermalC >= m_config.hotThresholdC)       { zone = TAMZone::Hot;         action = MemoryScheduleAction::LazyAlloc;      allocMB = m_config.lazyAllocMB;    delay = true; }
        else if (thermalC >= m_config.warmThresholdC)      { zone = TAMZone::Warm;        action = MemoryScheduleAction::PartialPrealloc; allocMB = m_config.partialPreallocMB; }
        else                                                { zone = TAMZone::Comfortable; action = MemoryScheduleAction::FullPrealloc;   allocMB = m_config.fullPreallocMB; }

        return { zone, action, allocMB, delay };
    }

    static std::string ZoneName(TAMZone z) noexcept {
        switch (z) {
        case TAMZone::Comfortable: return "Comfortable";
        case TAMZone::Warm:        return "Warm";
        case TAMZone::Hot:         return "Hot";
        case TAMZone::Critical:    return "Critical";
        }
        return "Unknown";
    }

    static std::string ActionName(MemoryScheduleAction a) noexcept {
        switch (a) {
        case MemoryScheduleAction::FullPrealloc:    return "FullPrealloc";
        case MemoryScheduleAction::PartialPrealloc: return "PartialPrealloc";
        case MemoryScheduleAction::LazyAlloc:       return "LazyAlloc";
        case MemoryScheduleAction::Emergency:       return "Emergency";
        }
        return "Unknown";
    }

    const ThermalMemoryConfig& Config() const noexcept { return m_config; }

private:
    ThermalMemoryConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
