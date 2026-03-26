// SecureDecodeContext.h — Sandbox Isolation for Untrusted Format Decoders
// Copyright (c) 2026 ExplorerLens Project
//
// Routes decoding of untrusted/complex formats through a restricted process
// via Windows Job Objects + AppContainer, preventing decoder CVEs from
// escaping to the host Explorer process.
//
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SandboxLevel : uint8_t {
    None        = 0,  // in-process (trusted formats: BMP, PNG)
    Restricted  = 1,  // same process, restricted token
    Isolated    = 2,  // child process + Job Object (complex formats)
    AppContainer = 3, // AppContainer + restricted capabilities (untrusted plugins)
};

struct SandboxPolicy {
    SandboxLevel level{SandboxLevel::None};
    uint32_t     timeoutMs{5000};      // kill sandbox after this
    size_t       maxOutputBytes{16 * 1024 * 1024}; // 16 MB
    bool         allowNetworkAccess{false};
    bool         allowRegistryWrite{false};
    bool         allowFileSystemWrite{false};
};

struct SecureDecodeResult {
    bool                 success{false};
    std::vector<uint8_t> pixels;
    uint32_t             width{0};
    uint32_t             height{0};
    uint32_t             stride{0};
    uint32_t             exitCode{0};
    std::string          errorMessage;
    double               elapsedMs{0.0};
};

class SecureDecodeContext {
public:
    explicit SecureDecodeContext(SandboxPolicy policy = {});
    ~SecureDecodeContext();

    SecureDecodeContext(const SecureDecodeContext&) = delete;
    SecureDecodeContext& operator=(const SecureDecodeContext&) = delete;

    // Decode file data inside the configured sandbox.
    [[nodiscard]] SecureDecodeResult Decode(
        const std::string& extension,
        const void* data, size_t size,
        uint32_t reqWidth = 256,
        uint32_t reqHeight = 256);

    // Lookup the recommended sandbox level for a given file extension.
    static SandboxLevel RecommendedLevel(const std::string& ext) noexcept;

    // Return the active policy.
    [[nodiscard]] const SandboxPolicy& Policy() const noexcept { return m_policy; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    SandboxPolicy         m_policy;
};

// Extension → sandbox level mapping registry
struct FormatSandboxRule {
    const char*  extension;
    SandboxLevel level;
};

// Formats requiring elevated sandbox due to historical CVE exposure
inline constexpr FormatSandboxRule g_sandboxRules[] = {
    { ".pdf",  SandboxLevel::Isolated    },
    { ".svg",  SandboxLevel::Restricted  },
    { ".ai",   SandboxLevel::Isolated    },
    { ".eps",  SandboxLevel::Isolated    },
    { ".emf",  SandboxLevel::Restricted  },
    { ".wmf",  SandboxLevel::Restricted  },
    { ".wma",  SandboxLevel::Restricted  },
    { ".mp4",  SandboxLevel::Restricted  },
    { ".avi",  SandboxLevel::Restricted  },
    { ".mkv",  SandboxLevel::Restricted  },
    { ".7z",   SandboxLevel::Isolated    },
    { ".rar",  SandboxLevel::Isolated    },
    { ".zip",  SandboxLevel::Restricted  },
};

} // namespace Engine
} // namespace ExplorerLens
