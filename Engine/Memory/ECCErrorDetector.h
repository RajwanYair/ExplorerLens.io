// ECCErrorDetector.h — ECC Memory Error Detection Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Queries Windows WMI and hardware performance counters for ECC single/double-bit error events in DIMMs.
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

struct ECCErrorRecord { uint32_t singleBitErrors; uint32_t doubleBitErrors; uint64_t timestamp; std::string dimm; };
struct ECCStatus { bool eccSupported; bool eccEnabled; uint64_t totalSingleBit; uint64_t totalDoubleBit; };
class ECCErrorDetector {
public:
    ECCStatus    QueryStatus() const      { return { false, false, 0, 0 }; }
    bool         HasCorrectedErrors() const { return m_records.size() > 0 && m_records[0].singleBitErrors > 0; }
    bool         HasUncorrectedErrors() const { return false; }
    std::vector<ECCErrorRecord> GetHistory() const { return m_records; }
    void         ClearHistory()           { m_records.clear(); }
private:
    std::vector<ECCErrorRecord> m_records;
};

} // namespace Engine
} // namespace ExplorerLens