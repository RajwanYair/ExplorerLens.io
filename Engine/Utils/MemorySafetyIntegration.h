#pragma once
//==============================================================================
// MemorySafetyIntegration
// AddressSanitizer (ASAN) integration + memory-mapped I/O for large files
//
// Features:
// 1. ASAN build configuration generation for MSVC
// 2. Memory-mapped file reader for efficient large file access
// 3. Safe buffer operations with bounds checking
// 4. Memory leak detection helpers
// 5. Stack buffer overflow protection validation
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens { namespace Engine {

/// ASAN detection mode
enum class SanitizerMode : uint8_t {
 None, ///< No sanitizer enabled
 AddressSanitizer, ///< ASAN (heap/stack/global overflow)
 MemorySanitizer, ///< MSAN (uninitialized reads)
 UndefinedBehavior,///< UBSAN 
 ThreadSanitizer ///< TSAN (data races)
};

/// Memory access pattern for mapped I/O
enum class AccessPattern : uint8_t {
 Sequential, ///< Read from start to end
 Random, ///< Random access throughout
 HeaderOnly, ///< Only read first N bytes 
 Streaming ///< Large streaming reads
};

/// Memory-mapped file information
struct MappedFileInfo {
 std::wstring filePath;
 uint64_t fileSize = 0;
 uint64_t mappedSize = 0;
 const uint8_t* baseAddress = nullptr;
 AccessPattern pattern = AccessPattern::Sequential;
 bool readOnly = true;
 bool isValid = false;
};

/// ASAN build configuration
struct ASANBuildConfig {
 bool enableASAN = false;
 bool enableStackProtection = true;
 bool enableHeapProtection = true;
 bool enableGlobalProtection = true;
 bool enableLeakDetection = true;
 std::wstring suppressionFile;
 uint32_t quarantineSizeMB = 256;
 
 /// Generate MSVC compiler flags
 std::wstring GetCompilerFlags() const {
 std::wstring flags;
 if (enableASAN)
 flags = L"/fsanitize=address";
 return flags;
 }
 
 /// Generate CMake options
 std::wstring GetCMakeOptions() const {
 if (!enableASAN) return L"";
 return L"-DCMAKE_CXX_FLAGS=\"/fsanitize=address\" "
 L"-DCMAKE_C_FLAGS=\"/fsanitize=address\"";
 }
};

/// Safe buffer with automatic bounds checking
struct SafeBuffer {
 std::vector<uint8_t> data;
 uint64_t readPosition = 0;
 
 /// Create with specific size
 explicit SafeBuffer(size_t size) : data(size, 0) {}
 SafeBuffer() = default;
 
 /// Read bytes safely with bounds check
 bool Read(void* dst, size_t count) {
 if (readPosition + count > data.size())
 return false;
 std::memcpy(dst, data.data() + readPosition, count);
 readPosition += count;
 return true;
 }
 
 /// Read a typed value safely
 template<typename T>
 bool ReadValue(T& value) {
 return Read(&value, sizeof(T));
 }
 
 /// Check available bytes
 size_t Available() const {
 return (readPosition < data.size()) ? (data.size() - readPosition) : 0;
 }
 
 /// Seek to position
 bool Seek(uint64_t pos) {
 if (pos > data.size()) return false;
 readPosition = pos;
 return true;
 }
 
 /// Get total size
 size_t Size() const { return data.size(); }
 
 /// Check if valid
 bool IsValid() const { return !data.empty(); }
};

/// Memory safety diagnostic result
struct MemorySafetyReport {
 SanitizerMode mode = SanitizerMode::None;
 uint32_t totalAllocations = 0;
 uint32_t freedAllocations = 0;
 uint32_t leakedAllocations = 0;
 uint64_t leakedBytes = 0;
 uint32_t bufferOverflows = 0;
 uint32_t useAfterFree = 0;
 uint32_t stackOverflows = 0;
 bool passed = true;
 std::vector<std::wstring> findings;
};

//==============================================================================
// MemorySafetyIntegration
//==============================================================================
class MemorySafetyIntegration {
public:
 MemorySafetyIntegration();
 ~MemorySafetyIntegration() = default;

 /// Detect if ASAN is currently active in this build
 static bool IsASANEnabled();

 /// Get recommended ASAN build configuration
 static ASANBuildConfig GetRecommendedConfig();

 /// Create a memory-mapped file for efficient reading
 MappedFileInfo MapFile(const std::wstring& filePath,
 AccessPattern pattern = AccessPattern::Sequential) const;

 /// Unmap a previously mapped file
 static void UnmapFile(MappedFileInfo& info);

 /// Create a safe buffer from file data
 static SafeBuffer CreateSafeBuffer(const uint8_t* data, size_t size);

 /// Validate that a pointer range is within a safe buffer
 static bool ValidateAccess(const SafeBuffer& buffer, size_t offset, size_t length);

 /// Check for common memory safety issues in a decoder test
 MemorySafetyReport RunDecoderSafetyCheck(const std::wstring& decoderName) const;

 /// Get sanitizer mode name
 static const wchar_t* GetSanitizerName(SanitizerMode mode);

 /// Get access pattern name
 static const wchar_t* GetAccessPatternName(AccessPattern pattern);

 /// Get maximum safe file size for memory mapping (conservative)
 static uint64_t GetMaxMappableSize();

private:
 /// Check if file size is safe to map
 static bool IsSafeToMap(uint64_t fileSize);
};

}} // namespace ExplorerLens::Engine

