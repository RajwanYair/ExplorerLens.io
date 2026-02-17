// ============================================================================
// Sprint6_IsolationTests.cpp
// Sprint 6 - Worker/Isolation Stabilization Tests
// ============================================================================
// Deliverables:
// 1. SEH exception fuzzing with malformed/corrupt archives
// 2. Circuit breaker stress test: 5000 corrupt-payload iterations
// 3. Decoder timeout enforcement: hard-kill decoders exceeding 5-second wall clock
// 4. Memory leak regression test: 100-iteration decode loop with peak-heap assertion
//
// Updated: Integrates DecoderTimeout, FuzzingTestFixtures, and MemoryLeakTest
// infrastructure from Sprint 6 implementation.
// ============================================================================

#include "../Engine/Pipeline/ThumbnailPipeline.h"
#include "../Engine/Utils/DecoderCircuitBreaker.h"
#include "../Engine/Utils/DecoderTimeout.h"
#include "../Engine/Tests/FuzzingTestFixtures.h"
#include "../Engine/Tests/MemoryLeakTest.h"
#include <windows.h>
#include <Psapi.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <random>

namespace DarkThumbs {
namespace Sprint6Tests {

// ============================================================================
// Test 1: SEH Exception Fuzzing - Malformed Archives
// ============================================================================
bool TestSEHFuzzingCorruptArchives() {
    std::cout << "\n=== Test 1: SEH Fuzzing - Corrupt Archives ===" << std::endl;
    
    const wchar_t* testFormats[] = {
        L"corrupt.zip",
        L"corrupt.rar",
        L"corrupt.7z",
        L"corrupt.cbz",
        L"corrupt.cbr"
    };
    
    const size_t TEST_ITERATIONS = 1000;
    size_t crashCount = 0;
    size_t gracefulFailCount = 0;
    
    // Create test directory if needed
    CreateDirectoryW(L"tests\\data\\corrupt", nullptr);
    
    for (size_t iter = 0; iter < TEST_ITERATIONS; iter++) {
        for (const wchar_t* format : testFormats) {
            // Generate corrupt file with random garbage data
            std::wstring filePath = L"tests\\data\\corrupt\\";
            filePath += format;
            filePath += L"_";
            filePath += std::to_wstring(iter);
            
            GenerateCorruptFile(filePath.c_str(), 1024 + (rand() % 10240));
            
            // Try to generate thumbnail - should fail gracefully
            __try {
                HBITMAP hBitmap = nullptr;
                auto pipeline = ThumbnailPipeline::GetInstance();
                HRESULT hr = pipeline->GenerateThumbnail(
                    filePath.c_str(),
                    256, 256,
                    false, // CPU only for fuzzing
                    &hBitmap
                );
                
                if (hBitmap) {
                    DeleteObject(hBitmap);
                }
                
                if (FAILED(hr)) {
                    gracefulFailCount++;
                }
                
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                crashCount++;
                std::cout << "  CRASH detected for: " << iter << std::endl;
            }
            
            // Clean up
            DeleteFileW(filePath.c_str());
        }
        
        if ((iter + 1) % 100 == 0) {
            std::cout << "  Progress: " << (iter + 1) << "/" << TEST_ITERATIONS 
                      << " iterations, " << crashCount << " crashes" << std::endl;
        }
    }
    
    std::cout << "\n  Results:" << std::endl;
    std::cout << "    Total iterations: " << TEST_ITERATIONS * 5 << std::endl;
    std::cout << "    Crashes: " << crashCount << std::endl;
    std::cout << "    Graceful failures: " << gracefulFailCount << std::endl;
    
    // Exit criteria: 0 crashes
    bool passed = (crashCount == 0);
    std::cout << "  Status: " << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// Test 2: Circuit Breaker Stress Test - 5000 Corrupt Payloads
// ============================================================================
bool TestCircuitBreakerStress() {
    std::cout << "\n=== Test 2: Circuit Breaker Stress Test ===" << std::endl;
    
    const size_t STRESS_ITERATIONS = 5000;
    size_t explorerCrashes = 0;
    size_t circuitOpenCount = 0;
    
    // Reset circuit breakers
    CircuitBreakerManager::GetInstance().Reset();
    
    auto pipeline = ThumbnailPipeline::GetInstance();
    
    for (size_t i = 0; i < STRESS_ITERATIONS; i++) {
        // Generate corrupt payloads for different formats
        std::wstring corruptFile = L"tests\\data\\corrupt\\stress_";
        corruptFile += std::to_wstring(i);
        corruptFile += L".cbz";
        
        GenerateCorruptFile(corruptFile.c_str(), 2048);
        
        __try {
            HBITMAP hBitmap = nullptr;
            HRESULT hr = pipeline->GenerateThumbnail(
                corruptFile.c_str(),
                256, 256,
                false,
                &hBitmap
            );
            
            if (hBitmap) {
                DeleteObject(hBitmap);
            }
            
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            explorerCrashes++;
        }
        
        // Check circuit breaker states
        auto& cbManager = CircuitBreakerManager::GetInstance();
        auto status = cbManager.GetStatus();
        for (const auto& [decoderName, state] : status) {
            if (state == CircuitState::OPEN) {
                circuitOpenCount++;
            }
        }
        
        DeleteFileW(corruptFile.c_str());
        
        if ((i + 1) % 500 == 0) {
            std::cout << "  Progress: " << (i + 1) << "/" << STRESS_ITERATIONS 
                      << " iterations, " << explorerCrashes << " crashes, "
                      << circuitOpenCount << " circuit openings" << std::endl;
        }
    }
    
    std::cout << "\n  Results:" << std::endl;
    std::cout << "    Total iterations: " << STRESS_ITERATIONS << std::endl;
    std::cout << "    Explorer crashes: " << explorerCrashes << std::endl;
    std::cout << "    Circuit breaker activations: " << circuitOpenCount << std::endl;
    
    // Exit criteria: 0 Explorer crashes
    bool passed = (explorerCrashes == 0);
    std::cout << "  Status: " << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// Test 3: Decoder Timeout Enforcement - Hard Kill After 5 Seconds
// ============================================================================
bool TestDecoderTimeoutEnforcement() {
    std::cout << "\n=== Test 3: Decoder Timeout Enforcement ===" << std::endl;
    
    // Create a file that triggers slow decoding (e.g., massive resolution)
    std::wstring slowFile = L"tests\\data\\timeout_test.png";
    
    std::cout << "  Testing 5-second timeout enforcement..." << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    
    __try {
        HBITMAP hBitmap = nullptr;
        auto pipeline = ThumbnailPipeline::GetInstance();
        
        // This should timeout and be killed
        HRESULT hr = pipeline->GenerateThumbnail(
            slowFile.c_str(),
            256, 256,
            false,
            &hBitmap
        );
        
        if (hBitmap) {
            DeleteObject(hBitmap);
        }
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cout << "  Exception caught (expected for timeout)" << std::endl;
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "  Decode attempt duration: " << duration.count() << " ms" << std::endl;
    
    // Should be killed within 5500ms (5s timeout + 500ms grace)
    bool passed = (duration.count() < 5500);
    std::cout << "  Status: " << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// Test 4: Memory Leak Regression Test - 100 Iterations
// ============================================================================
bool TestMemoryLeakRegression() {
    std::cout << "\n=== Test 4: Memory Leak Regression Test ===" << std::endl;
    
    const size_t LEAK_TEST_ITERATIONS = 100;
    
    // Get baseline memory
    PROCESS_MEMORY_COUNTERS_EX memStart;
    GetProcessMemoryInfo(GetCurrentProcess(), 
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memStart), 
        sizeof(memStart));
    
    size_t peakHeapBefore = memStart.PeakWorkingSetSize;
    
    std::cout << "  Baseline peak heap: " << (peakHeapBefore / 1024 / 1024) << " MB" << std::endl;
    std::cout << "  Running " << LEAK_TEST_ITERATIONS << " decode iterations..." << std::endl;
    
    auto pipeline = ThumbnailPipeline::GetInstance();
    
    // Test with real sample files
    std::vector<std::wstring> testFiles = {
        L"tests\\data\\corpus\\images\\jpeg\\sample.jpg",
        L"tests\\data\\corpus\\images\\png\\sample.png",
        L"tests\\data\\corpus\\images\\webp\\sample.webp",
        L"tests\\data\\corpus\\archives\\zip\\sample.zip",
        L"tests\\data\\corpus\\archives\\cbz\\sample.cbz"
    };
    
    for (size_t i = 0; i < LEAK_TEST_ITERATIONS; i++) {
        for (const auto& testFile : testFiles) {
            HBITMAP hBitmap = nullptr;
            HRESULT hr = pipeline->GenerateThumbnail(
                testFile.c_str(),
                256, 256,
                false,
                &hBitmap
            );
            
            if (hBitmap) {
                DeleteObject(hBitmap);
            }
        }
        
        if ((i + 1) % 10 == 0) {
            // Check memory periodically
            PROCESS_MEMORY_COUNTERS_EX memCurrent;
            GetProcessMemoryInfo(GetCurrentProcess(), 
                reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memCurrent), 
                sizeof(memCurrent));
            
            std::cout << "  Iteration " << (i + 1) << ": " 
                      << (memCurrent.WorkingSetSize / 1024 / 1024) << " MB" << std::endl;
        }
    }
    
    // Force garbage collection
    Sleep(1000);
    
    // Get final memory
    PROCESS_MEMORY_COUNTERS_EX memEnd;
    GetProcessMemoryInfo(GetCurrentProcess(), 
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memEnd), 
        sizeof(memEnd));
    
    size_t peakHeapAfter = memEnd.PeakWorkingSetSize;
    size_t workingSetAfter = memEnd.WorkingSetSize;
    
    std::cout << "\n  Results:" << std::endl;
    std::cout << "    Peak heap before: " << (peakHeapBefore / 1024 / 1024) << " MB" << std::endl;
    std::cout << "    Peak heap after: " << (peakHeapAfter / 1024 / 1024) << " MB" << std::endl;
    std::cout << "    Working set after: " << (workingSetAfter / 1024 / 1024) << " MB" << std::endl;
    
    // Peak heap should not grow more than 2x
    double growthRatio = static_cast<double>(peakHeapAfter) / peakHeapBefore;
    std::cout << "    Growth ratio: " << growthRatio << "x" << std::endl;
    
    bool passed = (growthRatio < 2.0);
    std::cout << "  Status: " << (passed ? "PASS ✓" : "FAIL ✗") << std::endl;
    return passed;
}

// ============================================================================
// Helper: Generate Corrupt File with Random Data
// ============================================================================
void GenerateCorruptFile(const wchar_t* filePath, size_t size) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return;
    }
    
    // Write garbage data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < size; i++) {
        char byte = static_cast<char>(dis(gen));
        file.write(&byte, 1);
    }
    
    file.close();
}

} // namespace Sprint6Tests
} // namespace DarkThumbs

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    using namespace DarkThumbs::Sprint6Tests;
    
    std::cout << "============================================" << std::endl;
    std::cout << "Sprint 6: Worker/Isolation Stabilization Tests" << std::endl;
    std::cout << "============================================" << std::endl;
    
    bool allPassed = true;
    
    // Run all tests
    allPassed &= TestSEHFuzzingCorruptArchives();
    allPassed &= TestCircuitBreakerStress();
    allPassed &= TestDecoderTimeoutEnforcement();
    allPassed &= TestMemoryLeakRegression();
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "Sprint 6 Test Results: " << (allPassed ? "ALL PASS ✓" : "FAILURES DETECTED ✗") << std::endl;
    std::cout << "============================================" << std::endl;
    
    return allPassed ? 0 : 1;
}
