// DecoderRegistryTests.cpp — Catch2 tests for DecoderRegistryV2
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the DecoderRegistryV2 singleton: registration, discovery, priority
// ordering, enable/disable, extension lookup, and thread-safety invariants.
//
// Because DecoderRegistryV2 is a process-wide singleton, each test uses a
// unique decoder name (prefixed with "T<N>_") so TEST_CASE invocations do
// not pollute each other's state.  Tests that mutate shared state use
// RAII cleanup guards at the end of each TEST_CASE body.
//
// ROADMAP: §10.4 (Catch2 migration), D43 (FormatDetector pure-library design).
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "../../Core/DecoderRegistryV2.h"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// ============================================================================
// Minimal stub decoder used across all tests
// ============================================================================

namespace {

using namespace ExplorerLens::Engine;

/// Minimal IFormatDecoder stub — never decodes; only services registration.
class StubDecoder final : public IFormatDecoder
{
  public:
    explicit StubDecoder(std::string name, DecoderCaps caps = DecoderCaps::Decode)
        : m_name(std::move(name)), m_caps(caps) {}

    RegistryDecoderInfo GetInfo() const override {
        RegistryDecoderInfo info;
        info.name         = m_name;
        info.displayName  = m_name;
        info.capabilities = m_caps;
        info.source       = DecoderSource::BuiltIn;
        info.priority     = 100;
        return info;
    }

    bool CanDecode(const wchar_t*, const uint8_t*, size_t) override { return true; }
    HBITMAP Decode(const wchar_t*, uint32_t) override { return nullptr; }

  private:
    std::string  m_name;
    DecoderCaps  m_caps;
};

/// Helper: make a factory that returns a StubDecoder with the given name.
inline DecoderCreator MakeFactory(std::string name,
                                   DecoderCaps caps = DecoderCaps::Decode) {
    return [n = std::move(name), caps]() -> DecoderPtr {
        return std::make_shared<StubDecoder>(n, caps);
    };
}

/// RAII guard that removes test decoders when it goes out of scope.
struct RegistryCleanup {
    std::vector<std::string> names;
    ~RegistryCleanup() {
        auto& reg = DecoderRegistryV2::Instance();
        for (const auto& n : names)
            reg.UnregisterDecoder(n.c_str());
    }
};

} // anonymous namespace

// ============================================================================
// Basic registration tests
// ============================================================================

TEST_CASE("DecoderRegistryV2 — RegisterDecoder adds a decoder",
          "[registry][register]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    const auto before = reg.GetDecoderCount();
    REQUIRE(reg.RegisterDecoder("T01_STUB", MakeFactory("T01_STUB"),
                                 {".t01"}, 100));
    cleanup.names.push_back("T01_STUB");

    REQUIRE(reg.GetDecoderCount() == before + 1);
    REQUIRE(reg.IsExtensionSupported(".t01"));
}

TEST_CASE("DecoderRegistryV2 — double RegisterDecoder is rejected",
          "[registry][register]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T02_DUP", MakeFactory("T02_DUP"), {".t02"}, 100));
    cleanup.names.push_back("T02_DUP");

    // Second registration with same name must return false
    REQUIRE_FALSE(reg.RegisterDecoder("T02_DUP", MakeFactory("T02_DUP"), {".t02"}, 200));
}

TEST_CASE("DecoderRegistryV2 — UnregisterDecoder removes decoder and extension",
          "[registry][unregister]") {
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T03_REM", MakeFactory("T03_REM"), {".t03"}, 100));
    REQUIRE(reg.IsExtensionSupported(".t03"));

    REQUIRE(reg.UnregisterDecoder("T03_REM"));
    REQUIRE_FALSE(reg.IsExtensionSupported(".t03"));
}

TEST_CASE("DecoderRegistryV2 — UnregisterDecoder returns false for unknown name",
          "[registry][unregister]") {
    auto& reg = DecoderRegistryV2::Instance();
    REQUIRE_FALSE(reg.UnregisterDecoder("T04_NONEXISTENT_XYZ"));
}

// ============================================================================
// Discovery tests
// ============================================================================

TEST_CASE("DecoderRegistryV2 — FindBestDecoder returns null for unsupported extension",
          "[registry][discovery]") {
    auto& reg = DecoderRegistryV2::Instance();
    REQUIRE(reg.FindBestDecoder(".t05_nosuchfmt") == nullptr);
}

TEST_CASE("DecoderRegistryV2 — FindBestDecoder returns registered decoder",
          "[registry][discovery]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T06_FOUND", MakeFactory("T06_FOUND"), {".t06"}, 100));
    cleanup.names.push_back("T06_FOUND");

    auto dec = reg.FindBestDecoder(".t06");
    REQUIRE(dec != nullptr);
}

TEST_CASE("DecoderRegistryV2 — FindBestDecoder selects highest-priority decoder",
          "[registry][discovery][priority]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    // Register two decoders for the same extension with different priorities
    REQUIRE(reg.RegisterDecoder("T07_LOW",  MakeFactory("T07_LOW"),  {".t07"}, 50));
    REQUIRE(reg.RegisterDecoder("T07_HIGH", MakeFactory("T07_HIGH"), {".t07"}, 200));
    cleanup.names.push_back("T07_LOW");
    cleanup.names.push_back("T07_HIGH");

    auto dec = reg.FindBestDecoder(".t07");
    REQUIRE(dec != nullptr);
    // The returned decoder should be the high-priority one
    REQUIRE(dec->GetInfo().name == "T07_HIGH");
}

TEST_CASE("DecoderRegistryV2 — FindDecoders returns all decoders sorted by priority",
          "[registry][discovery][priority]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T08_P100", MakeFactory("T08_P100"), {".t08"}, 100));
    REQUIRE(reg.RegisterDecoder("T08_P300", MakeFactory("T08_P300"), {".t08"}, 300));
    REQUIRE(reg.RegisterDecoder("T08_P200", MakeFactory("T08_P200"), {".t08"}, 200));
    cleanup.names.push_back("T08_P100");
    cleanup.names.push_back("T08_P300");
    cleanup.names.push_back("T08_P200");

    auto decoders = reg.FindDecoders(".t08");
    REQUIRE(decoders.size() == 3);

    // Must be priority-descending: 300, 200, 100
    REQUIRE(decoders[0]->GetInfo().name == "T08_P300");
    REQUIRE(decoders[1]->GetInfo().name == "T08_P200");
    REQUIRE(decoders[2]->GetInfo().name == "T08_P100");
}

TEST_CASE("DecoderRegistryV2 — FindDecoders returns empty for unknown extension",
          "[registry][discovery]") {
    auto& reg = DecoderRegistryV2::Instance();
    REQUIRE(reg.FindDecoders(".t09_nosuchfmt").empty());
}

// ============================================================================
// Enable / disable tests
// ============================================================================

TEST_CASE("DecoderRegistryV2 — disabled decoder is excluded from FindBestDecoder",
          "[registry][enable-disable]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T10_MAIN", MakeFactory("T10_MAIN"), {".t10"}, 100));
    cleanup.names.push_back("T10_MAIN");

    // Disable and confirm lookup returns null
    reg.SetDecoderEnabled("T10_MAIN", false);
    REQUIRE(reg.FindBestDecoder(".t10") == nullptr);

    // Re-enable and confirm lookup succeeds
    reg.SetDecoderEnabled("T10_MAIN", true);
    REQUIRE(reg.FindBestDecoder(".t10") != nullptr);
}

TEST_CASE("DecoderRegistryV2 — FindBestDecoder falls back to second decoder when first is disabled",
          "[registry][enable-disable][priority]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T11_HIGH",    MakeFactory("T11_HIGH"),    {".t11"}, 500));
    REQUIRE(reg.RegisterDecoder("T11_FALLBCK", MakeFactory("T11_FALLBCK"), {".t11"}, 50));
    cleanup.names.push_back("T11_HIGH");
    cleanup.names.push_back("T11_FALLBCK");

    // Disable high-priority decoder — fallback must take over
    reg.SetDecoderEnabled("T11_HIGH", false);
    auto dec = reg.FindBestDecoder(".t11");
    REQUIRE(dec != nullptr);
    REQUIRE(dec->GetInfo().name == "T11_FALLBCK");

    reg.SetDecoderEnabled("T11_HIGH", true);  // restore (cleanup will also run)
}

// ============================================================================
// GetAllDecoders / count invariants
// ============================================================================

TEST_CASE("DecoderRegistryV2 — GetAllDecoders includes registered decoders",
          "[registry][introspection]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T12_INFO", MakeFactory("T12_INFO"), {".t12"}, 100));
    cleanup.names.push_back("T12_INFO");

    auto all = reg.GetAllDecoders();
    bool found = false;
    for (const auto& info : all) {
        if (info.name == "T12_INFO") { found = true; break; }
    }
    REQUIRE(found);
}

TEST_CASE("DecoderRegistryV2 — GetDecoderCount increments on register, decrements on unregister",
          "[registry][introspection]") {
    auto& reg = DecoderRegistryV2::Instance();

    const auto before = reg.GetDecoderCount();
    REQUIRE(reg.RegisterDecoder("T13_CNT", MakeFactory("T13_CNT"), {".t13"}, 100));

    REQUIRE(reg.GetDecoderCount() == before + 1);
    REQUIRE(reg.UnregisterDecoder("T13_CNT"));
    REQUIRE(reg.GetDecoderCount() == before);
}

// ============================================================================
// Extension case-insensitivity
// ============================================================================

TEST_CASE("DecoderRegistryV2 — extension lookup is case-insensitive",
          "[registry][extension][case]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    REQUIRE(reg.RegisterDecoder("T14_CASE", MakeFactory("T14_CASE"), {".T14"}, 100));
    cleanup.names.push_back("T14_CASE");

    REQUIRE(reg.IsExtensionSupported(".T14"));
    REQUIRE(reg.IsExtensionSupported(".t14"));
    REQUIRE(reg.FindBestDecoder(".T14") != nullptr);
    REQUIRE(reg.FindBestDecoder(".t14") != nullptr);
}

// ============================================================================
// Capability introspection
// ============================================================================

TEST_CASE("DecoderRegistryV2 — registered caps are retrievable from GetInfo",
          "[registry][caps]") {
    RegistryCleanup cleanup;
    auto& reg = DecoderRegistryV2::Instance();

    const auto caps = DecoderCaps::Decode | DecoderCaps::Metadata | DecoderCaps::AlphaChannel;
    REQUIRE(reg.RegisterDecoder("T15_CAPS", MakeFactory("T15_CAPS", caps), {".t15"}, 100, caps));
    cleanup.names.push_back("T15_CAPS");

    auto dec = reg.FindBestDecoder(".t15");
    REQUIRE(dec != nullptr);
    const auto info = dec->GetInfo();
    REQUIRE(HasCap(info.capabilities, DecoderCaps::Decode));
    REQUIRE(HasCap(info.capabilities, DecoderCaps::Metadata));
    REQUIRE(HasCap(info.capabilities, DecoderCaps::AlphaChannel));
    REQUIRE_FALSE(HasCap(info.capabilities, DecoderCaps::GPUAccelerated));
}

// ============================================================================
// Thread-safety smoke test
// ============================================================================

TEST_CASE("DecoderRegistryV2 — concurrent registrations do not crash",
          "[registry][thread-safety]") {
    auto& reg = DecoderRegistryV2::Instance();
    constexpr int kThreads = 4;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            // Each thread registers its own unique decoder
            std::string name = "T16_THREAD_" + std::to_string(i);
            std::string ext  = ".t16t" + std::to_string(i);
            if (reg.RegisterDecoder(name.c_str(), MakeFactory(name), {ext}, 100))
                ++successCount;
        });
    }
    for (auto& t : threads) t.join();

    // All registrations must have succeeded (unique names/exts)
    REQUIRE(successCount == kThreads);

    // Cleanup
    for (int i = 0; i < kThreads; ++i) {
        std::string name = "T16_THREAD_" + std::to_string(i);
        reg.UnregisterDecoder(name.c_str());
    }
}
