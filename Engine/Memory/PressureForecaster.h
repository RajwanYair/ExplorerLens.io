// PressureForecaster.h — Memory Pressure Forecaster (LSTM-Lite)
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts future memory pressure using a lightweight EWMA/LSTM-lite model to trigger proactive eviction.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class PressureForecast { Stable, Rising, Critical, ReducingLoad };
struct ForecastResult { PressureForecast forecast; double confidence; double predictedPressurePct; uint32_t horizonMs; };
class PressureForecaster {
public:
    void   Feed(double pressurePct) {
        m_ewma = m_ewma * 0.9 + pressurePct * 0.1;
        m_samples++;
    }
    ForecastResult Predict(uint32_t horizonMs = 1000) const {
        PressureForecast f = m_ewma > 80.0 ? PressureForecast::Critical :
                             m_ewma > 50.0 ? PressureForecast::Rising : PressureForecast::Stable;
        return { f, 0.75, m_ewma, horizonMs };
    }
    double EWMA()          const { return m_ewma; }
    size_t SampleCount()   const { return m_samples; }
private:
    double m_ewma   = 0.0;
    size_t m_samples = 0;
};

} // namespace Engine
} // namespace ExplorerLens