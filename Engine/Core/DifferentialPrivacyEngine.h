// DifferentialPrivacyEngine.h — Differential Privacy Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Applies (epsilon, delta)-differential privacy to usage telemetry before aggregation.
//
#pragma once
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct DPParameters
{
    double epsilon = 1.0;
    double delta = 1e-5;
    double sensitivity = 1.0;
};

struct DPQueryResult
{
    double noisyValue = 0.0;
    double privacyBudgetUsed = 0.0;
    bool budgetExceeded = false;
};

class DifferentialPrivacyEngine
{
  public:
    explicit DifferentialPrivacyEngine(const DPParameters& params) : m_params(params), m_budgetUsed(0.0), m_rng(42) {}

    DPQueryResult Query(double trueValue)
    {
        DPQueryResult r;
        double remaining = m_params.epsilon - m_budgetUsed;
        r.budgetExceeded = remaining <= 0.0;
        if (r.budgetExceeded) {
            r.noisyValue = 0.0;
            r.privacyBudgetUsed = m_budgetUsed;
            return r;
        }
        double scale = m_params.sensitivity / std::max(remaining, 1e-9);
        // Laplace noise via difference of two Exponential(1/scale) variates
        std::exponential_distribution<double> expDist(1.0 / scale);
        double laplaceNoise = expDist(m_rng) - expDist(m_rng);
        r.noisyValue = trueValue + laplaceNoise;
        m_budgetUsed += std::min(remaining, m_params.epsilon * 0.1);
        r.privacyBudgetUsed = m_budgetUsed;
        return r;
    }
    double RemainingBudget() const
    {
        return std::max(0.0, m_params.epsilon - m_budgetUsed);
    }
    void ResetBudget()
    {
        m_budgetUsed = 0.0;
    }
    const DPParameters& Parameters() const
    {
        return m_params;
    }

  private:
    DPParameters m_params;
    double m_budgetUsed;
    std::mt19937_64 m_rng;
};

}  // namespace Engine
}  // namespace ExplorerLens
