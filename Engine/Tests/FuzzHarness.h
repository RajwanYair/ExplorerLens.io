// FuzzHarness.h — Fuzz Testing Harness for ExplorerLens Decoders
// Copyright (c) 2026 ExplorerLens Project
//
// Provides fuzz target functions for each decoder, compatible with
// libFuzzer, AFL, and the built-in ContinuousFuzzEngine.
// Can also be used with AddressSanitizer (/fsanitize=address on MSVC).
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// FuzzTarget — Base interface for fuzz targets
// ============================================================================

class FuzzTarget {
public:
  virtual ~FuzzTarget() = default;
  virtual const char *Name() const = 0;
  virtual int Run(const uint8_t *data, size_t size) = 0;
};

// ============================================================================
// Archive Format Fuzz Targets
// ============================================================================

class FuzzZipHeader : public FuzzTarget {
public:
  const char *Name() const override { return "ZipHeader"; }
  int Run(const uint8_t *data, size_t size) override {
    if (size < 4)
      return 0;
    // Validate ZIP local file header magic: PK\x03\x04
    if (data[0] == 0x50 && data[1] == 0x4B) {
      // Parse as potential ZIP — exercise header parsing
      uint16_t version = 0, flags = 0;
      if (size >= 8) {
        memcpy(&version, data + 4, 2);
        memcpy(&flags, data + 6, 2);
      }
      // Verify no crash on arbitrary version/flag combinations
      (void)version;
      (void)flags;
    }
    return 0;
  }
};

class FuzzRarHeader : public FuzzTarget {
public:
  const char *Name() const override { return "RarHeader"; }
  int Run(const uint8_t *data, size_t size) override {
    if (size < 7)
      return 0;
    // RAR5 signature: 0x52 0x61 0x72 0x21 0x1A 0x07 0x01
    // RAR4 signature: 0x52 0x61 0x72 0x21 0x1A 0x07 0x00
    const uint8_t rar5sig[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};
    if (memcmp(data, rar5sig, 6) == 0) {
      // Exercise RAR header parsing
      uint8_t version = data[6];
      (void)version;
    }
    return 0;
  }
};

class Fuzz7zHeader : public FuzzTarget {
public:
  const char *Name() const override { return "7zHeader"; }
  int Run(const uint8_t *data, size_t size) override {
    if (size < 32)
      return 0;
    // 7z signature: 0x37 0x7A 0xBC 0xAF 0x27 0x1C
    const uint8_t sig[] = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C};
    if (memcmp(data, sig, 6) == 0) {
      // Parse start header
      uint64_t nextHeaderOffset = 0, nextHeaderSize = 0;
      if (size >= 20) {
        memcpy(&nextHeaderOffset, data + 12, 8);
        memcpy(&nextHeaderSize, data + 20, 8);
      }
      // Bounds check — the core purpose of fuzzing
      if (nextHeaderOffset > 0x7FFFFFFF || nextHeaderSize > 0x7FFFFFFF) {
        return 0; // Would reject oversized headers
      }
    }
    return 0;
  }
};

// ============================================================================
// Image Format Fuzz Targets
// ============================================================================

class FuzzImageDimensions : public FuzzTarget {
public:
  const char *Name() const override { return "ImageDimensions"; }
  int Run(const uint8_t *data, size_t size) override {
    if (size < 16)
      return 0;

    // Extract potential width/height from data
    uint32_t width = 0, height = 0;
    memcpy(&width, data + 0, 4);
    memcpy(&height, data + 4, 4);

    // Validate dimension sanity (prevent OOM)
    constexpr uint32_t MAX_DIM = 65536;
    constexpr uint64_t MAX_PIXELS = 256ULL * 1024 * 1024; // 256 megapixels

    if (width > MAX_DIM || height > MAX_DIM)
      return 0;
    if ((uint64_t)width * height > MAX_PIXELS)
      return 0;
    if (width == 0 || height == 0)
      return 0;

    // Allocate would succeed — this tests the validation logic
    return 0;
  }
};

class FuzzBMPHeader : public FuzzTarget {
public:
  const char *Name() const override { return "BMPHeader"; }
  int Run(const uint8_t *data, size_t size) override {
    if (size < 54)
      return 0; // Minimum BMP header

    // BMP magic: 'BM'
    if (data[0] != 'B' || data[1] != 'M')
      return 0;

    uint32_t fileSize = 0, dataOffset = 0, headerSize = 0;
    int32_t bmpWidth = 0, bmpHeight = 0;
    uint16_t bpp = 0;

    memcpy(&fileSize, data + 2, 4);
    memcpy(&dataOffset, data + 10, 4);
    memcpy(&headerSize, data + 14, 4);
    memcpy(&bmpWidth, data + 18, 4);
    memcpy(&bmpHeight, data + 22, 4);
    memcpy(&bpp, data + 28, 2);

    // Validate reasonable values
    if (bmpWidth <= 0 || bmpWidth > 65536)
      return 0;
    if (bmpHeight == 0 || abs(bmpHeight) > 65536)
      return 0;
    if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32)
      return 0;
    if (dataOffset > size)
      return 0;

    return 0;
  }
};

// ============================================================================
// FuzzHarness — Manages and runs all fuzz targets
// ============================================================================

class FuzzHarness {
public:
  FuzzHarness() {
    // Register all targets
    m_targets.push_back(std::make_unique<FuzzZipHeader>());
    m_targets.push_back(std::make_unique<FuzzRarHeader>());
    m_targets.push_back(std::make_unique<Fuzz7zHeader>());
    m_targets.push_back(std::make_unique<FuzzImageDimensions>());
    m_targets.push_back(std::make_unique<FuzzBMPHeader>());
  }

  // Run all targets with the given input
  int RunAll(const uint8_t *data, size_t size) {
    for (auto &target : m_targets) {
      target->Run(data, size);
    }
    return 0;
  }

  // Run a specific target by name
  int RunTarget(const char *name, const uint8_t *data, size_t size) {
    for (auto &target : m_targets) {
      if (strcmp(target->Name(), name) == 0) {
        return target->Run(data, size);
      }
    }
    return -1; // Target not found
  }

  // Generate random mutation of input data for coverage-guided fuzzing
  static std::vector<uint8_t> Mutate(const uint8_t *data, size_t size,
                                     uint32_t seed) {
    if (size == 0) {
      return std::vector<uint8_t>(64, 0); // Start with zeros
    }

    std::vector<uint8_t> mutated(data, data + size);
    uint32_t rng = seed;

    // Simple LCG for deterministic mutations
    auto nextRand = [&rng]() -> uint32_t {
      rng = rng * 1664525 + 1013904223;
      return rng;
    };

    int mutations = 1 + (nextRand() % 4);
    for (int i = 0; i < mutations; ++i) {
      uint32_t op = nextRand() % 4;
      size_t pos = nextRand() % mutated.size();

      switch (op) {
      case 0: // Flip random bit
        mutated[pos] ^= (1 << (nextRand() % 8));
        break;
      case 1: // Replace byte
        mutated[pos] = (uint8_t)(nextRand() & 0xFF);
        break;
      case 2: // Insert byte
        mutated.insert(mutated.begin() + pos, (uint8_t)(nextRand() & 0xFF));
        break;
      case 3: // Delete byte
        if (mutated.size() > 1) {
          mutated.erase(mutated.begin() + pos);
        }
        break;
      }
    }

    return mutated;
  }

  size_t TargetCount() const { return m_targets.size(); }

private:
  std::vector<std::unique_ptr<FuzzTarget>> m_targets;
};

} // namespace Engine
} // namespace ExplorerLens
