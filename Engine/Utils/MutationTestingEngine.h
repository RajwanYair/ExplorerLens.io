// MutationTestingEngine.h — Mutation Testing Harness
// Copyright (c) 2026 ExplorerLens Project
//
// Stryker-style mutation testing that generates code mutants and validates test suite kill rates.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MutationOperator {
    ArithmeticReplace,
    ComparisonFlip,
    LogicalNegate,
    ReturnFalse,
    ReturnZero
};
struct MutantResult
{
    std::string location;
    MutationOperator op;
    bool killed;
};
struct MutationReport
{
    size_t total;
    size_t killed;
    size_t survived;
    double killRate() const
    {
        return total ? (double)killed / total : 0.0;
    }
};
class MutationTestingEngine
{
  public:
    void AddMutant(MutantResult r)
    {
        m_results.push_back(r);
    }
    MutationReport Summarize() const
    {
        size_t k = 0;
        for (auto& r : m_results)
            if (r.killed)
                k++;
        return {m_results.size(), k, m_results.size() - k};
    }
    void Reset()
    {
        m_results.clear();
    }

  private:
    std::vector<MutantResult> m_results;
};

}  // namespace Engine
}  // namespace ExplorerLens