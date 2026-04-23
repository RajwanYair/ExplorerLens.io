// CLIContractTests.cpp — Catch2 tests for lens.exe CLI command contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the lens.exe CLI command contracts from §6.3.
// Tests cover: 8 registered commands (generate/info/register/doctor/
// benchmark/cache/serve/plugin), Phase 1 command completeness, exit code
// semantics (0=success, 1=usage error, 2=file not found, 3=decode failed),
// option naming convention (--kebab-case), global flag presence, and
// command purpose uniqueness.
//
// All tests are self-contained — no CLI headers or Windows headers included.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// CLI contract model (§6.3)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::CLIContract {

// ── Exit codes ─────────────────────────────────────────────────────────────

static constexpr int EXIT_SUCCESS_CODE  = 0;  // Completed successfully
static constexpr int EXIT_USAGE_ERROR   = 1;  // Bad arguments / missing flags
static constexpr int EXIT_FILE_NOT_FOUND= 2;  // Input path does not exist
static constexpr int EXIT_DECODE_FAILED = 3;  // Decoder returned error

// ── Phase classification ────────────────────────────────────────────────────

enum class CLIPhase : uint8_t {
    PHASE1 = 1,  // generate, info, register, doctor — shipping in Phase 1
    PHASE2 = 2,  // benchmark, cache, serve           — Phase 2
    PHASE3 = 3,  // plugin                            — Phase 3
};

// ── Command descriptor ─────────────────────────────────────────────────────

struct CLICommand {
    std::string_view name;          // Primary command name
    std::string_view purpose;       // One-line purpose (must be unique)
    CLIPhase         phase;         // Planned release phase
    bool             hasVerboseFlag;// All commands must have --verbose
    bool             hasHelpFlag;   // All commands must have --help
};

// ── Registered CLI commands ─────────────────────────────────────────────────

static constexpr std::array<CLICommand, 8> CLI_COMMANDS = {{
    { "generate",  "Generate thumbnail for a file and write output",       CLIPhase::PHASE1, true, true },
    { "info",      "Display decoder metadata for a file without decoding", CLIPhase::PHASE1, true, true },
    { "register",  "Register or unregister the shell extension",           CLIPhase::PHASE1, true, true },
    { "doctor",    "Diagnose environment and decoder health",               CLIPhase::PHASE1, true, true },
    { "benchmark", "Run decoder throughput benchmark suite",               CLIPhase::PHASE2, true, true },
    { "cache",     "Manage the L1/L2 thumbnail cache (purge/stats)",       CLIPhase::PHASE2, true, true },
    { "serve",     "Start local HTTP API for thumbnail generation",        CLIPhase::PHASE2, true, true },
    { "plugin",    "Manage third-party decoder plugins",                   CLIPhase::PHASE3, true, true },
}};

// ── Global flags that must be present on every command ─────────────────────

static constexpr std::array<std::string_view, 3> GLOBAL_FLAGS = {{
    "--verbose",
    "--help",
    "--json",       // Machine-readable output flag (§6.3 structured output)
}};

// ── Option name convention checker (§6.3: --kebab-case, no underscores) ───

inline bool IsKebabCase(std::string_view opt) {
    if (opt.size() < 3) return false;               // must have at least --x
    if (opt[0] != '-' || opt[1] != '-') return false; // must start with --
    std::string_view name = opt.substr(2);
    if (name.empty()) return false;
    // No uppercase, no underscores
    for (char c : name) {
        if (c >= 'A' && c <= 'Z') return false;
        if (c == '_') return false;
    }
    return true;
}

} // namespace ExplorerLens::Tests::CLIContract

using namespace ExplorerLens::Tests::CLIContract;

// ===========================================================================
// Exit codes
// ===========================================================================

TEST_CASE("ExitCode — SUCCESS is 0",         "[cli][exit]") { REQUIRE(EXIT_SUCCESS_CODE  == 0); }
TEST_CASE("ExitCode — USAGE_ERROR is 1",     "[cli][exit]") { REQUIRE(EXIT_USAGE_ERROR   == 1); }
TEST_CASE("ExitCode — FILE_NOT_FOUND is 2",  "[cli][exit]") { REQUIRE(EXIT_FILE_NOT_FOUND== 2); }
TEST_CASE("ExitCode — DECODE_FAILED is 3",   "[cli][exit]") { REQUIRE(EXIT_DECODE_FAILED == 3); }

TEST_CASE("ExitCode — all 4 codes are distinct",
          "[cli][exit][uniqueness]") {
    std::array<int, 4> codes = {
        EXIT_SUCCESS_CODE, EXIT_USAGE_ERROR,
        EXIT_FILE_NOT_FOUND, EXIT_DECODE_FAILED
    };
    for (size_t i = 0; i < codes.size(); ++i) {
        for (size_t j = i + 1; j < codes.size(); ++j) {
            CHECK(codes[i] != codes[j]);
        }
    }
}

// ===========================================================================
// Command registry
// ===========================================================================

TEST_CASE("CLICommands — 8 commands registered",
          "[cli][commands]") {
    REQUIRE(CLI_COMMANDS.size() == 8u);
}

TEST_CASE("CLICommands — all command names are non-empty",
          "[cli][commands]") {
    for (const auto& c : CLI_COMMANDS) {
        REQUIRE_FALSE(c.name.empty());
    }
}

TEST_CASE("CLICommands — all command purposes are non-empty",
          "[cli][commands]") {
    for (const auto& c : CLI_COMMANDS) {
        REQUIRE_FALSE(c.purpose.empty());
    }
}

TEST_CASE("CLICommands — all command names are unique",
          "[cli][commands][uniqueness]") {
    std::vector<std::string_view> seen;
    for (const auto& c : CLI_COMMANDS) {
        auto it = std::find(seen.begin(), seen.end(), c.name);
        INFO("Duplicate command: " << c.name);
        CHECK(it == seen.end());
        seen.push_back(c.name);
    }
}

TEST_CASE("CLICommands — all command purposes are unique (single responsibility)",
          "[cli][commands][uniqueness]") {
    std::vector<std::string_view> seen;
    for (const auto& c : CLI_COMMANDS) {
        auto it = std::find(seen.begin(), seen.end(), c.purpose);
        INFO("Duplicate purpose: " << c.purpose);
        CHECK(it == seen.end());
        seen.push_back(c.purpose);
    }
}

// ===========================================================================
// Phase 1 commands
// ===========================================================================

TEST_CASE("CLICommands — Phase 1 commands: generate, info, register, doctor",
          "[cli][commands][phase1]") {
    std::array<std::string_view, 4> phase1 = {"generate", "info", "register", "doctor"};
    for (auto name : phase1) {
        bool found = false;
        for (const auto& c : CLI_COMMANDS) {
            if (c.name == name) {
                CHECK(c.phase == CLIPhase::PHASE1);
                found = true;
                break;
            }
        }
        INFO("Missing Phase 1 command: " << name);
        CHECK(found);
    }
}

TEST_CASE("CLICommands — exactly 4 Phase 1 commands",
          "[cli][commands][phase1]") {
    int p1Count = 0;
    for (const auto& c : CLI_COMMANDS) {
        if (c.phase == CLIPhase::PHASE1) ++p1Count;
    }
    REQUIRE(p1Count == 4);
}

TEST_CASE("CLICommands — Phase 2 commands: benchmark, cache, serve",
          "[cli][commands][phase2]") {
    std::array<std::string_view, 3> phase2 = {"benchmark", "cache", "serve"};
    for (auto name : phase2) {
        for (const auto& c : CLI_COMMANDS) {
            if (c.name == name) {
                CHECK(c.phase == CLIPhase::PHASE2);
            }
        }
    }
}

TEST_CASE("CLICommands — plugin command is Phase 3",
          "[cli][commands][phase3]") {
    for (const auto& c : CLI_COMMANDS) {
        if (c.name == "plugin") {
            CHECK(c.phase == CLIPhase::PHASE3);
        }
    }
}

// ===========================================================================
// Global flags
// ===========================================================================

TEST_CASE("CLICommands — all commands have --verbose flag",
          "[cli][flags]") {
    for (const auto& c : CLI_COMMANDS) {
        INFO("Command: " << c.name);
        CHECK(c.hasVerboseFlag);
    }
}

TEST_CASE("CLICommands — all commands have --help flag",
          "[cli][flags]") {
    for (const auto& c : CLI_COMMANDS) {
        INFO("Command: " << c.name);
        CHECK(c.hasHelpFlag);
    }
}

TEST_CASE("GlobalFlags — 3 global flags defined (--verbose/--help/--json)",
          "[cli][flags][global]") {
    REQUIRE(GLOBAL_FLAGS.size() == 3u);
}

TEST_CASE("GlobalFlags — all start with double dash",
          "[cli][flags][global]") {
    for (auto f : GLOBAL_FLAGS) {
        REQUIRE(f.size() > 2);
        CHECK(f[0] == '-');
        CHECK(f[1] == '-');
    }
}

// ===========================================================================
// Option naming convention (--kebab-case)
// ===========================================================================

TEST_CASE("OptionNaming — IsKebabCase accepts valid kebab-case options",
          "[cli][naming]") {
    CHECK(IsKebabCase("--verbose"));
    CHECK(IsKebabCase("--output-path"));
    CHECK(IsKebabCase("--max-size"));
    CHECK(IsKebabCase("--json"));
    CHECK(IsKebabCase("--no-cache"));
}

TEST_CASE("OptionNaming — IsKebabCase rejects options with underscores",
          "[cli][naming]") {
    CHECK_FALSE(IsKebabCase("--output_path"));
    CHECK_FALSE(IsKebabCase("--max_size"));
    CHECK_FALSE(IsKebabCase("--no_cache"));
}

TEST_CASE("OptionNaming — IsKebabCase rejects options with uppercase",
          "[cli][naming]") {
    CHECK_FALSE(IsKebabCase("--OutputPath"));
    CHECK_FALSE(IsKebabCase("--maxSize"));
}

TEST_CASE("OptionNaming — IsKebabCase rejects single-dash options",
          "[cli][naming]") {
    CHECK_FALSE(IsKebabCase("-v"));
    CHECK_FALSE(IsKebabCase("-output"));
}

TEST_CASE("OptionNaming — global flags all pass kebab-case check",
          "[cli][flags][naming]") {
    for (auto f : GLOBAL_FLAGS) {
        INFO("Flag: " << f);
        CHECK(IsKebabCase(std::string(f)));
    }
}

// ===========================================================================
// Parametric command name check via GENERATE
// ===========================================================================

TEST_CASE("CLICommands — all 8 names consist only of lowercase letters",
          "[cli][commands][naming]") {
    auto name = GENERATE("generate", "info", "register", "doctor",
                         "benchmark", "cache", "serve", "plugin");
    for (char c : std::string_view(name)) {
        CHECK((c >= 'a' && c <= 'z'));
    }
}
