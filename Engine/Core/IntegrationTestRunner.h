#pragma once
/**
 * @file IntegrationTestRunner.h
 * @brief End-to-end integration test framework for the full thumbnail pipeline.
 * @version 15.0.0
 * @date 2026-03-02
 *
 * Provides a comprehensive integration test runner that validates the full
 * thumbnail decode pipeline from file input to RGBA output. Supports sequential
 * and parallel execution, timeout enforcement, automatic test-case discovery
 * from directories, and markdown report generation.
 *
 * @note Header-only. Uses Windows API + C++20 standard library only.
 * @warning Requires a user-provided decode callback to actually decode files.
 *
 * @copyright (c) 2026 ExplorerLens Contributors. All rights reserved.
 */

#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// @brief End-to-end integration test runner for thumbnail pipeline validation.
class IntegrationTestRunner {
public:
    /// @brief Describes a single test case.
    struct TestCase {
        std::wstring name;
        std::wstring inputFile;
        uint32_t expectedWidth = 0;
        uint32_t expectedHeight = 0;
        bool     shouldSucceed = true;
        uint32_t maxTimeMs = 5000;
    };

    /// @brief Result of executing a single test case.
    struct TestResult {
        std::wstring name;
        bool         passed = false;
        std::wstring error;
        uint32_t     actualWidth = 0;
        uint32_t     actualHeight = 0;
        uint32_t     timeMs = 0;
        size_t       outputSize = 0;
    };

    /// @brief Aggregate statistics for a test run.
    struct RunnerStats {
        uint32_t total = 0;
        uint32_t passed = 0;
        uint32_t failed = 0;
        uint32_t skipped = 0;
        uint32_t avgTimeMs = 0;
        uint32_t maxTimeMs = 0;
        uint32_t totalTimeMs = 0;
    };

    /// Decode callback: (inputFile, targetW, targetH, outRGBA, outW, outH) -> success
    using DecodeCallback = std::function<bool(const std::wstring&, uint32_t, uint32_t,
        std::vector<uint8_t>&, uint32_t&, uint32_t&)>;

    IntegrationTestRunner() noexcept {
        InitializeSRWLock(&m_lock);
    }

    ~IntegrationTestRunner() = default;

    // Non-copyable / non-movable due to SRWLOCK
    IntegrationTestRunner(const IntegrationTestRunner&) = delete;
    IntegrationTestRunner& operator=(const IntegrationTestRunner&) = delete;

    /// @brief Register the decode function under test.
    inline void SetDecodeCallback(DecodeCallback fn) noexcept {
        AcquireSRWLockExclusive(&m_lock);
        m_decodeCallback = std::move(fn);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// @brief Add a single test case.
    inline void AddTestCase(TestCase&& tc) {
        AcquireSRWLockExclusive(&m_lock);
        m_cases.emplace_back(std::move(tc));
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// @brief Auto-discover files in @p dir and create test cases for each.
    inline void AddTestCasesFromDirectory(const std::wstring& dir,
        uint32_t targetW,
        uint32_t targetH) {
        WIN32_FIND_DATAW fd{};
        std::wstring pattern = dir;
        if (!pattern.empty() && pattern.back() != L'\\' && pattern.back() != L'/') {
            pattern += L'\\';
        }
        pattern += L"*";

        HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return;

        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

            std::wstring fileName(fd.cFileName);
            std::wstring fullPath = dir;
            if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
                fullPath += L'\\';
            }
            fullPath += fileName;

            TestCase tc;
            tc.name = fileName;
            tc.inputFile = fullPath;
            tc.expectedWidth = targetW;
            tc.expectedHeight = targetH;
            tc.shouldSucceed = true;
            tc.maxTimeMs = 5000;

            AcquireSRWLockExclusive(&m_lock);
            m_cases.emplace_back(std::move(tc));
            ReleaseSRWLockExclusive(&m_lock);

        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }

    /// @brief Run all registered test cases sequentially.
    inline std::vector<TestResult> RunAll() {
        AcquireSRWLockShared(&m_lock);
        auto cases = m_cases;
        auto cb = m_decodeCallback;
        ReleaseSRWLockShared(&m_lock);

        std::vector<TestResult> results;
        results.reserve(cases.size());
        for (auto& tc : cases) {
            results.emplace_back(ExecuteTestCase(tc, cb));
        }
        return results;
    }

    /// @brief Run all registered test cases in parallel with @p threads worker threads.
    inline std::vector<TestResult> RunParallel(uint32_t threads = 4) {
        AcquireSRWLockShared(&m_lock);
        auto cases = m_cases;
        auto cb = m_decodeCallback;
        ReleaseSRWLockShared(&m_lock);

        threads = (std::max)(threads, 1u);
        threads = (std::min)(threads, static_cast<uint32_t>(cases.size()));

        std::vector<TestResult> results(cases.size());
        std::atomic<size_t> nextIdx{ 0 };

        auto worker = [&]() {
            while (true) {
                size_t idx = nextIdx.fetch_add(1, std::memory_order_relaxed);
                if (idx >= cases.size()) break;
                results[idx] = ExecuteTestCase(cases[idx], cb);
            }
            };

        std::vector<std::thread> pool;
        pool.reserve(threads);
        for (uint32_t i = 0; i < threads; ++i) {
            pool.emplace_back(worker);
        }
        for (auto& t : pool) {
            if (t.joinable()) t.join();
        }
        return results;
    }

    /// @brief Generate a markdown report from test results.
    inline std::wstring GenerateReport(const std::vector<TestResult>& results) const {
        auto stats = ComputeStats(results);

        std::wostringstream ws;
        ws << L"# Integration Test Report\n\n";
        ws << L"**Date:** " << GetTimestampW() << L"\n\n";
        ws << L"## Summary\n\n";
        ws << L"| Metric | Value |\n";
        ws << L"|--------|-------|\n";
        ws << L"| Total  | " << stats.total << L" |\n";
        ws << L"| Passed | " << stats.passed << L" |\n";
        ws << L"| Failed | " << stats.failed << L" |\n";
        ws << L"| Avg Time (ms) | " << stats.avgTimeMs << L" |\n";
        ws << L"| Max Time (ms) | " << stats.maxTimeMs << L" |\n";
        ws << L"| Total Time (ms) | " << stats.totalTimeMs << L" |\n\n";

        ws << L"## Results\n\n";
        ws << L"| Test | Status | Width | Height | Time (ms) | Output Size |\n";
        ws << L"|------|--------|-------|--------|-----------|-------------|\n";

        for (auto& r : results) {
            ws << L"| " << r.name
                << L" | " << (r.passed ? L"PASS" : L"**FAIL**")
                << L" | " << r.actualWidth
                << L" | " << r.actualHeight
                << L" | " << r.timeMs
                << L" | " << r.outputSize
                << L" |\n";
        }

        // Failures detail
        bool anyFailed = false;
        for (auto& r : results) {
            if (!r.passed) {
                if (!anyFailed) {
                    ws << L"\n## Failures\n\n";
                    anyFailed = true;
                }
                ws << L"### " << r.name << L"\n";
                ws << L"- **Error:** " << r.error << L"\n";
                ws << L"- **Time:** " << r.timeMs << L" ms\n\n";
            }
        }
        return ws.str();
    }

    /// @brief Save the report to a file.
    inline bool SaveReport(const std::wstring& path,
        const std::vector<TestResult>& results) const {
        std::wstring report = GenerateReport(results);

        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        // Write UTF-8 BOM + content
        std::string utf8 = WideToUTF8(report);
        const uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
        DWORD written = 0;
        WriteFile(hFile, bom, 3, &written, nullptr);
        WriteFile(hFile, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr);
        CloseHandle(hFile);
        return true;
    }

    /// @brief Compute aggregate stats from results.
    inline RunnerStats GetStats(const std::vector<TestResult>& results) const {
        return ComputeStats(results);
    }

private:
    SRWLOCK          m_lock{};
    std::vector<TestCase> m_cases;
    DecodeCallback   m_decodeCallback;

    /// Execute a single test case with timeout enforcement.
    inline TestResult ExecuteTestCase(const TestCase& tc, const DecodeCallback& cb) const {
        TestResult result;
        result.name = tc.name;

        if (!cb) {
            result.passed = false;
            result.error = L"No decode callback set";
            return result;
        }

        // Verify file exists and can be opened
        HANDLE hFile = CreateFileW(tc.inputFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            result.passed = !tc.shouldSucceed;
            result.error = tc.shouldSucceed ? L"File not found or inaccessible" : L"Expected failure: file inaccessible";
            return result;
        }
        CloseHandle(hFile);

        // Run decode with timing
        auto start = std::chrono::steady_clock::now();

        std::vector<uint8_t> outputRGBA;
        uint32_t outW = 0, outH = 0;

        // Use a separate thread for timeout enforcement
        std::atomic<bool> completed{ false };
        std::atomic<bool> decodeOk{ false };

        std::thread decodeThread([&]() {
            bool ok = false;
            try {
                ok = cb(tc.inputFile, tc.expectedWidth, tc.expectedHeight, outputRGBA, outW, outH);
            }
            catch (...) {
                ok = false;
            }
            decodeOk.store(ok, std::memory_order_release);
            completed.store(true, std::memory_order_release);
            });

        // Wait up to maxTimeMs
        auto deadline = std::chrono::steady_clock::now() +
            std::chrono::milliseconds(tc.maxTimeMs);

        while (!completed.load(std::memory_order_acquire)) {
            if (std::chrono::steady_clock::now() >= deadline) {
                // Timeout — we can't forcibly kill the thread, but we note it
                result.error = L"Timeout exceeded (" + std::to_wstring(tc.maxTimeMs) + L" ms)";
                result.passed = false;
                if (decodeThread.joinable()) decodeThread.detach();
                auto end = std::chrono::steady_clock::now();
                result.timeMs = static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
                return result;
            }
            Sleep(1);
        }

        if (decodeThread.joinable()) decodeThread.join();

        auto end = std::chrono::steady_clock::now();
        result.timeMs = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        bool ok = decodeOk.load(std::memory_order_acquire);

        if (tc.shouldSucceed) {
            if (!ok) {
                result.passed = false;
                result.error = L"Decode returned failure";
            }
            else {
                result.actualWidth = outW;
                result.actualHeight = outH;
                result.outputSize = outputRGBA.size();

                // Validate RGBA size = W * H * 4
                size_t expectedSize = static_cast<size_t>(outW) * outH * 4;
                if (outputRGBA.size() != expectedSize) {
                    result.passed = false;
                    result.error = L"RGBA buffer size mismatch: expected " +
                        std::to_wstring(expectedSize) + L" got " +
                        std::to_wstring(outputRGBA.size());
                }
                else if (tc.expectedWidth > 0 && outW != tc.expectedWidth) {
                    result.passed = false;
                    result.error = L"Width mismatch: expected " +
                        std::to_wstring(tc.expectedWidth) + L" got " +
                        std::to_wstring(outW);
                }
                else if (tc.expectedHeight > 0 && outH != tc.expectedHeight) {
                    result.passed = false;
                    result.error = L"Height mismatch: expected " +
                        std::to_wstring(tc.expectedHeight) + L" got " +
                        std::to_wstring(outH);
                }
                else {
                    result.passed = true;
                }
            }
        }
        else {
            // Should fail
            result.passed = !ok;
            if (ok) {
                result.error = L"Expected failure but decode succeeded";
            }
        }
        return result;
    }

    /// Compute aggregate statistics.
    inline RunnerStats ComputeStats(const std::vector<TestResult>& results) const {
        RunnerStats s;
        s.total = static_cast<uint32_t>(results.size());
        uint64_t sumTime = 0;
        for (auto& r : results) {
            if (r.passed) ++s.passed; else ++s.failed;
            sumTime += r.timeMs;
            s.maxTimeMs = (std::max)(s.maxTimeMs, r.timeMs);
        }
        s.totalTimeMs = static_cast<uint32_t>(sumTime);
        s.avgTimeMs = s.total > 0 ? static_cast<uint32_t>(sumTime / s.total) : 0;
        return s;
    }

    /// Get current timestamp as wide string.
    static inline std::wstring GetTimestampW() {
        SYSTEMTIME st{};
        GetLocalTime(&st);
        std::wostringstream ws;
        ws << std::setfill(L'0')
            << st.wYear << L'-'
            << std::setw(2) << st.wMonth << L'-'
            << std::setw(2) << st.wDay << L' '
            << std::setw(2) << st.wHour << L':'
            << std::setw(2) << st.wMinute << L':'
            << std::setw(2) << st.wSecond;
        return ws.str();
    }

    /// Convert wide string to UTF-8 using WideCharToMultiByte.
    static inline std::string WideToUTF8(const std::wstring& wide) {
        if (wide.empty()) return {};
        int len = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
            static_cast<int>(wide.size()),
            nullptr, 0, nullptr, nullptr);
        if (len <= 0) return {};
        std::string out(static_cast<size_t>(len), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.data(),
            static_cast<int>(wide.size()),
            out.data(), len, nullptr, nullptr);
        return out;
    }
};

} // namespace Engine
} // namespace ExplorerLens
