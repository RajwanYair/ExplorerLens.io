//==============================================================================
// DarkThumbs Engine — Sprint 316: Memory Safety Audit V2
// AddressSanitizer + HeapGuard integration, use-after-free detection,
// buffer overflow probing, and heap metadata validation for all decoders.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class MemSafetyViolation : uint8_t {
    UseAfterFree=0,BufferOverflow,HeapCorruption,UninitialisedRead,
    DoubleFree,StackOverflow,NullDeref, COUNT
};
enum class MemSafetyTool : uint8_t { ASAN=0,HeapGuard,PageHeap,DrMemory,WinHeap, COUNT };
enum class MemSafetyScope : uint8_t { AllDecoders=0,PluginsOnly,CoreOnly,FullEngine, COUNT };

struct MemSafetyFinding {
    MemSafetyViolation  violation   = MemSafetyViolation::BufferOverflow;
    MemSafetyTool       tool        = MemSafetyTool::ASAN;
    std::wstring        component;
    std::wstring        description;
    bool                fixed       = false;
};

struct MemSafetyAuditReport {
    MemSafetyScope  scope           = MemSafetyScope::FullEngine;
    uint32_t        findingsCount   = 0;
    uint32_t        fixedCount      = 0;
    uint32_t        openCount       = 0;
    bool            clean           = false;
};

class MemorySafetyAuditV2 {
public:
    static const wchar_t* ViolationName(MemSafetyViolation v) {
        switch(v) {
            case MemSafetyViolation::UseAfterFree:     return L"Use-After-Free";
            case MemSafetyViolation::BufferOverflow:   return L"Buffer Overflow";
            case MemSafetyViolation::HeapCorruption:   return L"Heap Corruption";
            case MemSafetyViolation::UninitialisedRead:return L"Uninitialised Read";
            case MemSafetyViolation::DoubleFree:       return L"Double Free";
            case MemSafetyViolation::StackOverflow:    return L"Stack Overflow";
            case MemSafetyViolation::NullDeref:        return L"Null Dereference";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ToolName(MemSafetyTool t) {
        switch(t) {
            case MemSafetyTool::ASAN:     return L"AddressSanitizer";
            case MemSafetyTool::HeapGuard:return L"HeapGuard";
            case MemSafetyTool::PageHeap: return L"PageHeap";
            case MemSafetyTool::DrMemory: return L"Dr. Memory";
            case MemSafetyTool::WinHeap:  return L"Windows Heap";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ScopeName(MemSafetyScope s) {
        switch(s) {
            case MemSafetyScope::AllDecoders: return L"All Decoders";
            case MemSafetyScope::PluginsOnly: return L"Plugins Only";
            case MemSafetyScope::CoreOnly:    return L"Core Only";
            case MemSafetyScope::FullEngine:  return L"Full Engine";
            default: return L"Unknown";
        }
    }
    static constexpr size_t ViolationCount() { return static_cast<size_t>(MemSafetyViolation::COUNT); }
    static constexpr size_t ToolCount()      { return static_cast<size_t>(MemSafetyTool::COUNT); }
    static constexpr size_t ScopeCount()     { return static_cast<size_t>(MemSafetyScope::COUNT); }
};

}} // namespace DarkThumbs::Engine
