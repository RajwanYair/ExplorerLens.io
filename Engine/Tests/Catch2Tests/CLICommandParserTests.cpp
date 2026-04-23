// CLICommandParserTests.cpp — Catch2 tests for CLI types and struct defaults
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the CLI type system from LensCLI.h and CLICommandParser.h:
//   - CLIExitCode enum values and integer mapping
//   - CLICommand enum membership
//   - GenerateOptions default values (§lens.exe --generate contract)
//   - BatchOptions default values
//   - TokenType enum membership from CLICommandParser.h
//
// Does NOT call Parse() which requires wchar_t* argv[] — tested at integration
// level. This file validates the type contracts that callers depend on.
//
// References: ROADMAP §lens.exe CLI spec, §10.2 test stack, §15.1 P0 CLI.
//
#include <catch2/catch_test_macros.hpp>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../CLI/LensCLI.h"
#include "../../CLI/CLICommandParser.h"

using namespace ExplorerLens::Engine;

// =============================================================================
// §1 — CLIExitCode integer values (contract: lens.exe process exit codes)
// =============================================================================

TEST_CASE("CLIExitCode: Success == 0", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::Success) == 0);
}

TEST_CASE("CLIExitCode: GeneralError == 1", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::GeneralError) == 1);
}

TEST_CASE("CLIExitCode: InvalidArgs == 2", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::InvalidArgs) == 2);
}

TEST_CASE("CLIExitCode: FormatUnknown == 3", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::FormatUnknown) == 3);
}

TEST_CASE("CLIExitCode: LicenseError == 4", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::LicenseError) == 4);
}

TEST_CASE("CLIExitCode: IOError == 5", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::IOError) == 5);
}

TEST_CASE("CLIExitCode: PartialFailure == 6", "[cli][exitcode]") {
    REQUIRE(static_cast<int>(CLIExitCode::PartialFailure) == 6);
}

TEST_CASE("CLIExitCode: no two codes share the same integer", "[cli][exitcode]") {
    // Verify the enum is densely packed 0–6 with no collisions
    REQUIRE(static_cast<int>(CLIExitCode::Success)        != static_cast<int>(CLIExitCode::GeneralError));
    REQUIRE(static_cast<int>(CLIExitCode::GeneralError)   != static_cast<int>(CLIExitCode::InvalidArgs));
    REQUIRE(static_cast<int>(CLIExitCode::InvalidArgs)    != static_cast<int>(CLIExitCode::FormatUnknown));
    REQUIRE(static_cast<int>(CLIExitCode::FormatUnknown)  != static_cast<int>(CLIExitCode::LicenseError));
    REQUIRE(static_cast<int>(CLIExitCode::LicenseError)   != static_cast<int>(CLIExitCode::IOError));
    REQUIRE(static_cast<int>(CLIExitCode::IOError)        != static_cast<int>(CLIExitCode::PartialFailure));
}

// =============================================================================
// §2 — CLICommand enum: all expected values compile and are distinct
// =============================================================================

TEST_CASE("CLICommand: None is a valid command value", "[cli][command]") {
    CLICommand cmd = CLICommand::None;
    REQUIRE(cmd == CLICommand::None);
}

TEST_CASE("CLICommand: Generate is distinct from None", "[cli][command]") {
    REQUIRE(CLICommand::Generate != CLICommand::None);
}

TEST_CASE("CLICommand: Batch is distinct from Generate", "[cli][command]") {
    REQUIRE(CLICommand::Batch != CLICommand::Generate);
}

TEST_CASE("CLICommand: Cache, Info, Formats, Version, Help are all distinct", "[cli][command]") {
    REQUIRE(CLICommand::Cache   != CLICommand::Info);
    REQUIRE(CLICommand::Formats != CLICommand::Version);
    REQUIRE(CLICommand::Version != CLICommand::Help);
    REQUIRE(CLICommand::Help    != CLICommand::Cache);
}

TEST_CASE("CLICommand: switch exhausts all 8 commands without warning", "[cli][command]") {
    // Compile-time completeness check
    auto classify = [](CLICommand c) -> const char* {
        switch (c) {
        case CLICommand::None:     return "none";
        case CLICommand::Generate: return "generate";
        case CLICommand::Batch:    return "batch";
        case CLICommand::Cache:    return "cache";
        case CLICommand::Info:     return "info";
        case CLICommand::Formats:  return "formats";
        case CLICommand::Version:  return "version";
        case CLICommand::Help:     return "help";
        }
        return "unknown";
    };
    REQUIRE(std::string(classify(CLICommand::Generate)) == "generate");
    REQUIRE(std::string(classify(CLICommand::Help))     == "help");
    REQUIRE(std::string(classify(CLICommand::None))     == "none");
}

// =============================================================================
// §3 — GenerateOptions: default values per §lens.exe contract
// =============================================================================

TEST_CASE("GenerateOptions: default width == 256", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE(opts.width == 256u);
}

TEST_CASE("GenerateOptions: default height == 256", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE(opts.height == 256u);
}

TEST_CASE("GenerateOptions: default timeoutMs == 15000", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE(opts.timeoutMs == 15000u);
}

TEST_CASE("GenerateOptions: default highQuality == false", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE_FALSE(opts.highQuality);
}

TEST_CASE("GenerateOptions: default forceCPU == false", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE_FALSE(opts.forceCPU);
}

TEST_CASE("GenerateOptions: default inputPath is empty", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE(opts.inputPath.empty());
}

TEST_CASE("GenerateOptions: default outputPath is empty (auto-derive from input)", "[cli][generate]") {
    GenerateOptions opts;
    REQUIRE(opts.outputPath.empty());
}

TEST_CASE("GenerateOptions: width/height can be set independently", "[cli][generate]") {
    GenerateOptions opts;
    opts.width  = 512;
    opts.height = 384;
    REQUIRE(opts.width  == 512u);
    REQUIRE(opts.height == 384u);
}

// =============================================================================
// §4 — BatchOptions: default values
// =============================================================================

TEST_CASE("BatchOptions: default width == 256", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.width == 256u);
}

TEST_CASE("BatchOptions: default height == 256", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.height == 256u);
}

TEST_CASE("BatchOptions: default threads == 0 (auto)", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.threads == 0u);
}

TEST_CASE("BatchOptions: default recursive == false", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE_FALSE(opts.recursive);
}

TEST_CASE("BatchOptions: default skipExisting == true", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.skipExisting);
}

TEST_CASE("BatchOptions: default progressBar == true", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.progressBar);
}

TEST_CASE("BatchOptions: default inputDir is empty", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.inputDir.empty());
}

TEST_CASE("BatchOptions: default filter is empty string", "[cli][batch]") {
    BatchOptions opts;
    REQUIRE(opts.filter.empty());
}

// =============================================================================
// §5 — TokenType enum from CLICommandParser.h
// =============================================================================

TEST_CASE("TokenType: Flag and ShortFlag are distinct", "[cli][token]") {
    REQUIRE(TokenType::Flag != TokenType::ShortFlag);
}

TEST_CASE("TokenType: Positional and KeyValue are distinct from Flag", "[cli][token]") {
    REQUIRE(TokenType::Positional != TokenType::Flag);
    REQUIRE(TokenType::KeyValue   != TokenType::Flag);
    REQUIRE(TokenType::Positional != TokenType::KeyValue);
}

// =============================================================================
// §6 — OptionDef struct defaults and usage
// =============================================================================

TEST_CASE("OptionDef: can construct with long and short names", "[cli][optiondef]") {
    OptionDef def;
    def.longName  = L"output";
    def.shortName = L'o';
    def.isFlag    = false;
    def.required  = false;
    REQUIRE(def.longName  == L"output");
    REQUIRE(def.shortName == L'o');
    REQUIRE_FALSE(def.isFlag);
    REQUIRE_FALSE(def.required);
}

TEST_CASE("OptionDef: required flag default", "[cli][optiondef]") {
    OptionDef def;
    REQUIRE_FALSE(def.required);
}
