// CacheMigrationManager.h — Storage tier cache migration
// Copyright (c) 2026 ExplorerLens Project
//
// Migrates cached thumbnails between storage tiers (RAM → SSD → HDD)
// based on access frequency and available capacity.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct CacheMigrationManagerConfig {
    bool enabled = true;
    uint64_t ramBudgetMB = 256;
    uint64_t ssdBudgetMB = 2048;
    uint32_t migrationBatchSize = 32;
    std::string label = "CacheMigrationManager";
};

class CacheMigrationManager {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheMigrationManagerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Tier : uint8_t { RAM, SSD, HDD };

    Tier SelectTier(uint32_t accessFrequency) const {
        if (accessFrequency >= 10) return Tier::RAM;
        if (accessFrequency >= 3)  return Tier::SSD;
        return Tier::HDD;
    }

    bool ShouldMigrate(Tier current, Tier target) const {
        return current != target;
    }

    void RecordMigration(Tier from, Tier to) {
        (void)from;
        m_totalMigrations++;
        if (to == Tier::RAM) m_promotions++;
        if (to == Tier::HDD) m_demotions++;
    }

    uint64_t GetTotalMigrations() const { return m_totalMigrations; }
    uint64_t GetPromotions() const { return m_promotions; }
    uint64_t GetDemotions() const { return m_demotions; }

private:
    bool m_initialized = false;
    CacheMigrationManagerConfig m_config;
    uint64_t m_totalMigrations = 0;
    uint64_t m_promotions = 0;
    uint64_t m_demotions = 0;
};

}
} // namespace ExplorerLens::Engine
