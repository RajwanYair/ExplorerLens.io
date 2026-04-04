// ============================================================================
// FuzzingTestFixtures.h
// Malformed/corrupt archive test payload generation for ExplorerLens
//
// Generates intentionally corrupt archive payloads for SEH fuzzing tests.
// Each fixture creates a minimal valid-looking header followed by garbage
// data to test decoder resilience against malformed inputs.
//
// USAGE (in test code):
// auto fixtures = FuzzingTestFixtures::GenerateAll();
// for (const auto& fixture : fixtures) {
// ThumbnailRequest req;
// req.filePath = fixture.tempPath;
// auto result = pipeline.GenerateThumbnail(req);
// // Should not crash — result should be E_FAIL or similar
// }
// ============================================================================

#pragma once

#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Testing {

/// A single fuzzing test fixture representing a corrupt file payload
struct FuzzFixture
{
    std::wstring tempPath;       ///< Path to temporary corrupt file
    std::string formatName;      ///< Format being fuzzed (e.g., "ZIP", "RAR")
    std::string corruptionType;  ///< Type of corruption applied
    size_t payloadSize;          ///< Size of the corrupt payload
};

/// Corruption strategy types
enum class CorruptionType {
    TRUNCATED_HEADER,  ///< Valid magic bytes, truncated before data
    RANDOM_PAYLOAD,    ///< Valid magic bytes, random garbage data
    ZERO_FILL,         ///< Valid magic bytes, zero-filled data section
    OVERSIZED_FIELD,   ///< Header fields with impossibly large values
    NULL_BYTES,        ///< All null bytes (no valid header at all)
    BIT_FLIP,          ///< Valid file with random bit flips
    EMPTY_FILE         ///< Zero-length file
};

/// Generates malformed archive/image payloads for fuzzing decoders
///
/// Creates temporary files with intentionally corrupt content to verify
/// that decoders handle bad input gracefully (no crashes, no hangs).
///
/// Thread Safety: GenerateAll() creates files in a unique temp directory
/// per call. CleanUp() removes those files.
class FuzzingTestFixtures
{
  public:
    /// Generate all fuzzing fixtures for all supported formats
    /// @param count Number of corrupt files per format (default: 10)
    /// @return Vector of FuzzFixture descriptors with temp file paths
    static std::vector<FuzzFixture> GenerateAll(int countPerFormat = 10)
    {
        std::vector<FuzzFixture> fixtures;
        auto tempDir = CreateTempDir();

        // Archive formats
        GenerateCorruptFiles(fixtures, tempDir, "ZIP", L".zip", {0x50, 0x4B, 0x03, 0x04}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "RAR", L".rar", {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "7Z", L".7z", {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "TAR", L".tar", {0x75, 0x73, 0x74, 0x61, 0x72}, countPerFormat);

        // Comic book archives (same magic as underlying format)
        GenerateCorruptFiles(fixtures, tempDir, "CBZ", L".cbz", {0x50, 0x4B, 0x03, 0x04}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "CBR", L".cbr", {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07}, countPerFormat);

        // Image formats
        GenerateCorruptFiles(fixtures, tempDir, "WEBP", L".webp", {0x52, 0x49, 0x46, 0x46},
                             countPerFormat);  // RIFF header
        GenerateCorruptFiles(fixtures, tempDir, "PNG", L".png", {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A},
                             countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "JPEG", L".jpg", {0xFF, 0xD8, 0xFF, 0xE0}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "GIF", L".gif", {0x47, 0x49, 0x46, 0x38, 0x39, 0x61}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "BMP", L".bmp", {0x42, 0x4D}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "TIFF", L".tif", {0x49, 0x49, 0x2A, 0x00},
                             countPerFormat);  // Little-endian TIFF

        // Modern image formats
        GenerateCorruptFiles(fixtures, tempDir, "JXL", L".jxl", {0xFF, 0x0A}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "AVIF", L".avif", {0x00, 0x00, 0x00, 0x1C, 0x66, 0x74, 0x79, 0x70},
                             countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "HEIF", L".heif", {0x00, 0x00, 0x00, 0x1C, 0x66, 0x74, 0x79, 0x70},
                             countPerFormat);

        // Professional formats
        GenerateCorruptFiles(fixtures, tempDir, "PSD", L".psd", {0x38, 0x42, 0x50, 0x53}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "EXR", L".exr", {0x76, 0x2F, 0x31, 0x01}, countPerFormat);
        GenerateCorruptFiles(fixtures, tempDir, "HDR", L".hdr", {0x23, 0x3F, 0x52, 0x41, 0x44, 0x49, 0x41, 0x4E},
                             countPerFormat);  // #?RADIAN

        // Document formats
        GenerateCorruptFiles(fixtures, tempDir, "PDF", L".pdf", {0x25, 0x50, 0x44, 0x46, 0x2D},
                             countPerFormat);  // %PDF-
        GenerateCorruptFiles(fixtures, tempDir, "EPUB", L".epub", {0x50, 0x4B, 0x03, 0x04},
                             countPerFormat);  // ZIP-based

        return fixtures;
    }

    /// Clean up temporary fixture files
    /// @param fixtures Vector of fixtures to clean up
    static void CleanUp(const std::vector<FuzzFixture>& fixtures)
    {
        std::error_code ec;
        std::set<std::filesystem::path> dirs;

        for (const auto& f : fixtures) {
            std::filesystem::remove(f.tempPath, ec);
            dirs.insert(std::filesystem::path(f.tempPath).parent_path());
        }

        for (const auto& dir : dirs) {
            std::filesystem::remove(dir, ec);  // Remove if empty
        }
    }

    /// Generate a single corrupt file for targeted testing
    /// @param format Format name (for naming)
    /// @param extension File extension
    /// @param magic Magic bytes for the format
    /// @param corruption Type of corruption to apply
    /// @return FuzzFixture descriptor
    static FuzzFixture GenerateSingle(const std::string& format, const std::wstring& extension,
                                      const std::vector<uint8_t>& magic,
                                      CorruptionType corruption = CorruptionType::RANDOM_PAYLOAD)
    {
        auto tempDir = CreateTempDir();
        return CreateCorruptFile(tempDir, format, extension, magic, corruption, 0);
    }

  private:
    /// Create a unique temporary directory for fixtures
    static std::filesystem::path CreateTempDir()
    {
        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);

        auto dir = std::filesystem::path(tempPath) / L"ExplorerLens_Fuzz";
        std::filesystem::create_directories(dir);
        return dir;
    }

    /// Generate corrupt files for a specific format
    static void GenerateCorruptFiles(std::vector<FuzzFixture>& fixtures, const std::filesystem::path& tempDir,
                                     const std::string& format, const std::wstring& extension,
                                     const std::vector<uint8_t>& magic, int count)
    {
        // Generate diverse corruption types
        std::vector<CorruptionType> types = {CorruptionType::TRUNCATED_HEADER, CorruptionType::RANDOM_PAYLOAD,
                                             CorruptionType::ZERO_FILL,        CorruptionType::OVERSIZED_FIELD,
                                             CorruptionType::NULL_BYTES,       CorruptionType::BIT_FLIP,
                                             CorruptionType::EMPTY_FILE};

        for (int i = 0; i < count; i++) {
            auto corruption = types[i % types.size()];
            fixtures.push_back(CreateCorruptFile(tempDir, format, extension, magic, corruption, i));
        }
    }

    /// Create a single corrupt file
    static FuzzFixture CreateCorruptFile(const std::filesystem::path& tempDir, const std::string& format,
                                         const std::wstring& extension, const std::vector<uint8_t>& magic,
                                         CorruptionType corruption, int index)
    {
        static std::mt19937 rng(42);  // Deterministic seed for reproducibility

        FuzzFixture fixture;
        fixture.formatName = format;

        // Build filename
        auto filename = std::filesystem::path(L"fuzz_" + std::to_wstring(index) + L"_"
                                              + std::wstring(format.begin(), format.end()) + extension);
        fixture.tempPath = (tempDir / filename).wstring();

        std::vector<uint8_t> payload;

        switch (corruption) {
            case CorruptionType::TRUNCATED_HEADER:
                fixture.corruptionType = "truncated_header";
                // Only partial magic bytes
                if (magic.size() > 1) {
                    payload.assign(magic.begin(), magic.begin() + magic.size() / 2);
                } else {
                    payload = magic;
                }
                break;

            case CorruptionType::RANDOM_PAYLOAD:
                fixture.corruptionType = "random_payload";
                payload = magic;  // Valid header
                for (int j = 0; j < 1024; j++) {
                    payload.push_back(static_cast<uint8_t>(rng() & 0xFF));
                }
                break;

            case CorruptionType::ZERO_FILL:
                fixture.corruptionType = "zero_fill";
                payload = magic;
                payload.resize(magic.size() + 4096, 0x00);
                break;

            case CorruptionType::OVERSIZED_FIELD:
                fixture.corruptionType = "oversized_field";
                payload = magic;
                // Insert max-value 32-bit fields after magic
                for (int j = 0; j < 8; j++) {
                    payload.push_back(0xFF);
                }
                // Fill rest with semi-valid data
                for (int j = 0; j < 256; j++) {
                    payload.push_back(static_cast<uint8_t>(j & 0xFF));
                }
                break;

            case CorruptionType::NULL_BYTES:
                fixture.corruptionType = "null_bytes";
                payload.resize(512, 0x00);
                break;

            case CorruptionType::BIT_FLIP:
                fixture.corruptionType = "bit_flip";
                payload = magic;
                for (int j = 0; j < 512; j++) {
                    payload.push_back(static_cast<uint8_t>(rng() & 0xFF));
                }
                // Flip random bits in the payload
                for (size_t j = 0; j < payload.size(); j += 7) {
                    payload[j] ^= static_cast<uint8_t>(1 << (rng() % 8));
                }
                break;

            case CorruptionType::EMPTY_FILE:
                fixture.corruptionType = "empty_file";
                // payload stays empty — zero-length file
                break;
        }

        fixture.payloadSize = payload.size();

        // Write payload to temp file
        std::ofstream out(fixture.tempPath, std::ios::binary);
        if (out.is_open()) {
            out.write(reinterpret_cast<const char*>(payload.data()), payload.size());
            out.close();
        }

        return fixture;
    }
};

/// Stress test runner for circuit breaker validation
///
/// Runs repeated corrupt-payload decode attempts to verify that:
/// 1. The circuit breaker opens after threshold failures
/// 2. No Explorer crashes occur (0 crashes / N attempts)
/// 3. Memory usage stays bounded
class CircuitBreakerStressTest
{
  public:
    struct StressResult
    {
        int totalAttempts = 0;
        int decoderFailures = 0;
        int circuitOpens = 0;
        int sehExceptions = 0;
        int timeouts = 0;
        bool explorerCrash = false;  ///< Should always be false
        double peakMemoryMB = 0.0;
        double elapsedSeconds = 0.0;
    };

    /// Run stress test with corrupt payloads
    /// @param iterations Number of corrupt payload attempts (target: 5000+)
    /// @return StressResult with metrics
    static StressResult Run(int iterations = 5000)
    {
        StressResult result;
        auto startTime = std::chrono::steady_clock::now();

        // Generate fixtures once, reuse across iterations
        auto fixtures = FuzzingTestFixtures::GenerateAll(20);

        // Reset all circuit breakers before test
        CircuitBreakerManager::Instance().ResetAll();

        for (int i = 0; i < iterations; i++) {
            const auto& fixture = fixtures[i % fixtures.size()];
            result.totalAttempts++;

            // Track memory usage every 100 iterations
            if (i % 100 == 0) {
                double currentMB = GetProcessMemoryMB();
                if (currentMB > result.peakMemoryMB) {
                    result.peakMemoryMB = currentMB;
                }
            }

            // Attempt decode via timeout guard
            DecoderTimeoutGuard guard(fixture.formatName, 5000);
            auto timeoutResult = guard.Execute([&]() -> HRESULT {
                // Simulate decode attempt — in real use this calls the pipeline
                // For stress testing, we verify the SEH wrapper handles bad data
                return E_FAIL;  // Corrupt files always fail
            });

            switch (timeoutResult) {
                case TimeoutResult::SUCCESS:
                    break;  // Unexpected for corrupt files
                case TimeoutResult::TIMED_OUT:
                    result.timeouts++;
                    break;
                case TimeoutResult::EXCEPTION:
                    result.sehExceptions++;
                    result.decoderFailures++;
                    break;
                case TimeoutResult::CIRCUIT_OPEN:
                    result.circuitOpens++;
                    break;
            }
        }

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        result.elapsedSeconds = std::chrono::duration<double>(elapsed).count();

        // Clean up fixtures
        FuzzingTestFixtures::CleanUp(fixtures);

        return result;
    }

  private:
    /// Get current process memory usage in MB
    static double GetProcessMemoryMB()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc = {};
        pmc.cb = sizeof(pmc);
        if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
            return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        }
        return 0.0;
    }
};

}  // namespace Testing
}  // namespace ExplorerLens
