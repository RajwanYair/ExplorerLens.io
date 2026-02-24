//==============================================================================
// ExplorerLens Engine — Parallel I/O Pipeline
// Overlapped async I/O with scatter-gather DMA staging, read-ahead
// distance tuning, I/O priority clamping, and per-volume queue depth
// optimization for thumbnail decode throughput.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class IOBackend    : uint8_t { Win32Overlapped=0, IOCP, WinRTAsync, DirectStorage, COUNT };
enum class IOPriority   : uint8_t { Critical=0,High,Normal,Low,Idle,COUNT };
enum class VolumeType   : uint8_t { NVMe=0,SATA_SSD,HDD,Network,RAM_Disk,COUNT };

struct ParallelIOConfig {
    IOBackend   backend         = IOBackend::IOCP;
    IOPriority  priority        = IOPriority::Normal;
    VolumeType  volumeType      = VolumeType::NVMe;
    uint8_t     queueDepth      = 32;    // max outstanding requests
    uint32_t    readAheadKB     = 256;   // read-ahead window (KB)
    bool        scatterGather   = true;
    bool        directStorage   = false; // requires DS GPU-enabled volume
};

struct ParallelIOStats {
    uint64_t   bytesRead        = 0;
    uint64_t   requestsIssued   = 0;
    uint64_t   requestsCompleted= 0;
    float      avgLatencyMs     = 0.0f;
    float      throughputMBps   = 0.0f;
    uint8_t    activeDepth      = 0;
};

class ParallelIOPipeline {
public:
    static const wchar_t* BackendName(IOBackend b) {
        switch(b) {
            case IOBackend::Win32Overlapped: return L"Win32 Overlapped";
            case IOBackend::IOCP:            return L"I/O Completion Port";
            case IOBackend::WinRTAsync:      return L"WinRT Async";
            case IOBackend::DirectStorage:   return L"DirectStorage";
            default: return L"Unknown";
        }
    }
    static const wchar_t* PriorityName(IOPriority p) {
        switch(p) {
            case IOPriority::Critical: return L"Critical";
            case IOPriority::High:     return L"High";
            case IOPriority::Normal:   return L"Normal";
            case IOPriority::Low:      return L"Low";
            case IOPriority::Idle:     return L"Idle";
            default: return L"Unknown";
        }
    }
    static const wchar_t* VolumeTypeName(VolumeType v) {
        switch(v) {
            case VolumeType::NVMe:      return L"NVMe SSD";
            case VolumeType::SATA_SSD:  return L"SATA SSD";
            case VolumeType::HDD:       return L"HDD";
            case VolumeType::Network:   return L"Network";
            case VolumeType::RAM_Disk:  return L"RAM Disk";
            default: return L"Unknown";
        }
    }
    static constexpr size_t BackendCount()    { return static_cast<size_t>(IOBackend::COUNT); }
    static constexpr size_t PriorityCount()   { return static_cast<size_t>(IOPriority::COUNT); }
    static constexpr size_t VolumeTypeCount() { return static_cast<size_t>(VolumeType::COUNT); }
};

}} // namespace ExplorerLens::Engine

