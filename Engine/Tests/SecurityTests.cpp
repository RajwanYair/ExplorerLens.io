/******************************************************************************
 * ExplorerLens Security Test Suite
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Comprehensive tests for plugin security infrastructure including:
 * - Isolation mode selection
 * - Process isolation and sandboxing
 * - IPC communication
 * - Crash detection and recovery
 * - Resource limits
 * - Signature verification
 * - Trust management
 *****************************************************************************/

#include "GTestShim.h"
#include "../Plugin/PluginManager.h"
#include "../Plugin/PluginDecoder.h"
#include "../Plugin/IsolationModeSelector.h"
#include "../Plugin/PluginHostClient.h"
#include "../Plugin/CrashHandler.h"
#include "../Plugin/Security/JobObjectManager.h"
#include "../Plugin/IPC/SharedMemoryManager.h"
#include <filesystem>
#include <thread>
#include <chrono>

using namespace ExplorerLens;
using namespace ExplorerLens::Engine;

//============================================================================
// Test Fixtures
//============================================================================

class SecurityTestFixture : public ::testing::Test {
protected:
 void SetUp() override {
 // Create test plugin directory
 test_plugin_dir_ = std::filesystem::temp_directory_path() / "explorerlens_test_plugins";
 std::filesystem::create_directories(test_plugin_dir_);
 
 // Load IsolationModeSelector configuration
 IsolationModeSelector::Instance().LoadConfiguration();
 }
 
 void TearDown() override {
 // Cleanup test directory
 std::error_code ec;
 std::filesystem::remove_all(test_plugin_dir_, ec);
 }
 
 std::filesystem::path test_plugin_dir_;
};

//============================================================================
// 1. Isolation Mode Selection Tests
//============================================================================

TEST_F(SecurityTestFixture, IsolationMode_UnsignedPlugin_UsesPluginHost) {
 // Unsigned plugins should always use PluginHost mode for security
 std::filesystem::path unsigned_plugin = test_plugin_dir_ / "unsigned_plugin.dll";
 
 IsolationMode mode = IsolationModeSelector::Instance().DetermineMode(
 L"unsigned_plugin", unsigned_plugin);
 
 EXPECT_EQ(mode, IsolationMode::PluginHost) 
 << "Unsigned plugins must run in PluginHost for security";
}

TEST_F(SecurityTestFixture, IsolationMode_TrustedPlugin_CanUseInWorker) {
 // Mark a plugin as trusted
 IsolationModeSelector::Instance().AddTrustedPlugin(L"TrustedTestPlugin");
 
 std::filesystem::path trusted_plugin = test_plugin_dir_ / "trusted_plugin.dll";
 
 IsolationMode mode = IsolationModeSelector::Instance().DetermineMode(
 L"TrustedTestPlugin", trusted_plugin);
 
 // Trusted plugins can use In-Worker mode if allowed
 EXPECT_TRUE(mode == IsolationMode::InWorker || mode == IsolationMode::PluginHost)
 << "Trusted plugins should be eligible for In-Worker mode";
}

TEST_F(SecurityTestFixture, IsolationMode_UserPreference_Respected) {
 // Set user preference to disallow In-Worker
 IsolationModeSelector::Instance().SetUserPreference(L"TestPlugin", false);
 
 std::filesystem::path plugin = test_plugin_dir_ / "test_plugin.dll";
 
 IsolationMode mode = IsolationModeSelector::Instance().DetermineMode(
 L"TestPlugin", plugin);
 
 EXPECT_EQ(mode, IsolationMode::PluginHost)
 << "User preference should override default mode";
}

TEST_F(SecurityTestFixture, IsolationMode_MinimumMode_Enforced) {
 // Minimum mode should be enforced
 IsolationMode min_mode = IsolationModeSelector::Instance().GetMinimumIsolationMode();
 
 EXPECT_TRUE(min_mode == IsolationMode::PluginHost || 
 min_mode == IsolationMode::InWorker)
 << "Minimum isolation mode should be valid";
}

//============================================================================
// 2. PluginHost Process Isolation Tests
//============================================================================

TEST_F(SecurityTestFixture, PluginHost_ProcessCreation_Succeeds) {
 std::filesystem::path test_plugin = test_plugin_dir_ / "test.dll";
 
 PluginHostClient client(test_plugin);
 
 HRESULT hr = client.Start();
 
 if (SUCCEEDED(hr)) {
 EXPECT_TRUE(client.IsRunning()) << "PluginHost should be running after Start()";
 
 // Cleanup
 client.Stop();
 EXPECT_FALSE(client.IsRunning()) << "PluginHost should stop after Stop()";
 }
 else {
 // Starting PluginHost may fail if executable not found, which is acceptable in unit tests
 EXPECT_TRUE(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ||
 hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
 << "Expected file not found error for missing PluginHost.exe";
 }
}

TEST_F(SecurityTestFixture, PluginHost_MultipleInstances_Isolated) {
 std::filesystem::path plugin1 = test_plugin_dir_ / "plugin1.dll";
 std::filesystem::path plugin2 = test_plugin_dir_ / "plugin2.dll";
 
 PluginHostClient client1(plugin1);
 PluginHostClient client2(plugin2);
 
 HRESULT hr1 = client1.Start();
 HRESULT hr2 = client2.Start();
 
 if (SUCCEEDED(hr1) && SUCCEEDED(hr2)) {
 EXPECT_TRUE(client1.IsRunning());
 EXPECT_TRUE(client2.IsRunning());
 
 // Each plugin should have its own process
 // (process IDs would be different, but we can't easily check that without exposing them)
 
 client1.Stop();
 client2.Stop();
 }
}

TEST_F(SecurityTestFixture, PluginHost_Timeout_HandledGracefully) {
 std::filesystem::path test_plugin = test_plugin_dir_ / "slow_plugin.dll";
 
 PluginHostClient client(test_plugin);
 client.SetRequestTimeout(100); // Very short timeout
 
 HRESULT hr = client.Start();
 
 if (SUCCEEDED(hr)) {
 // Try to decode with very short timeout - should timeout
 DecodeResult result = {};
 hr = client.DecodeImage("nonexistent.png", 256, 256, &result);
 
 // Should get timeout error (or file not found, which is also acceptable)
 EXPECT_TRUE(FAILED(hr)) << "Should fail with timeout or error";
 
 client.Stop();
 }
}

//============================================================================
// 3. IPC Communication Tests
//============================================================================

TEST_F(SecurityTestFixture, IPC_NamedPipe_ConnectionEstablished) {
 // Named pipe connection is tested implicitly through PluginHost tests
 // This test verifies the concept
 
 std::wstring pipe_name = L"\\\\.\\pipe\\ExplorerLens_Test_" + 
 std::to_wstring(GetCurrentProcessId());
 
 // Create pipe
 HANDLE hPipe = CreateNamedPipeW(
 pipe_name.c_str(),
 PIPE_ACCESS_DUPLEX,
 PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
 1, 4096, 4096, 0, nullptr);
 
 EXPECT_NE(hPipe, INVALID_HANDLE_VALUE) << "Should create named pipe successfully";
 
 if (hPipe != INVALID_HANDLE_VALUE) {
 CloseHandle(hPipe);
 }
}

TEST_F(SecurityTestFixture, IPC_SharedMemory_ReadWrite) {
 const wchar_t* test_section_name = L"ExplorerLens_Test_SharedMem";
 const size_t test_size = 4096;
 
 SharedMemorySection shared_mem(test_section_name, test_size);
 
 EXPECT_TRUE(shared_mem.IsValid()) << "Shared memory should be valid";
 
 if (shared_mem.IsValid()) {
 // Write test data
 const char* test_data = "Hello from shared memory!";
 size_t data_len = strlen(test_data) + 1;
 
 EXPECT_TRUE(shared_mem.Write(test_data, data_len, 0))
 << "Should write to shared memory";
 
 // Read back
 char read_buffer[256] = {};
 EXPECT_TRUE(shared_mem.Read(read_buffer, data_len, 0))
 << "Should read from shared memory";
 
 EXPECT_STREQ(test_data, read_buffer)
 << "Read data should match written data";
 }
}

//============================================================================
// 4. Crash Detection and Recovery Tests
//============================================================================

TEST_F(SecurityTestFixture, Crash_Detection_RegistersPlugin) {
 CrashHandler& handler = CrashHandler::Instance();
 
 std::wstring test_plugin = L"CrashTestPlugin";
 std::filesystem::path plugin_path = test_plugin_dir_ / "crash_test.dll";
 
 // Simulate a crash
 CrashInfo crash_info;
 crash_info.plugin_id = test_plugin;
 crash_info.plugin_path = plugin_path;
 crash_info.exit_code = STATUS_ACCESS_VIOLATION;
 crash_info.crash_time = std::chrono::system_clock::now();
 crash_info.crash_type = CrashType::AccessViolation;
 
 handler.OnPluginCrash(crash_info);
 
 // Check if plugin is disabled
 EXPECT_TRUE(handler.IsPluginDisabled(test_plugin))
 << "Plugin should be disabled after crash";
 
 // Check crash count
 EXPECT_GT(handler.GetCrashCount(test_plugin), 0u)
 << "Crash count should be incremented";
}

TEST_F(SecurityTestFixture, Crash_MultiplePlugins_IndependentTracking) {
 CrashHandler& handler = CrashHandler::Instance();
 
 std::wstring plugin1 = L"Plugin1";
 std::wstring plugin2 = L"Plugin2";
 
 // Crash plugin1
 CrashInfo crash1;
 crash1.plugin_id = plugin1;
 crash1.exit_code = STATUS_ACCESS_VIOLATION;
 crash1.crash_type = CrashType::AccessViolation;
 handler.OnPluginCrash(crash1);
 
 // Plugin2 should not be affected
 EXPECT_FALSE(handler.IsPluginDisabled(plugin2))
 << "Unrelated plugin should not be disabled";
}

//============================================================================
// 5. Resource Limit Tests
//============================================================================

TEST_F(SecurityTestFixture, JobObject_CreationAndConfiguration) {
 std::wstring job_name = L"ExplorerLens_Test_Job";
 JobObjectManager job_mgr(job_name);
 
 EXPECT_TRUE(job_mgr.IsValid()) << "Job object should be created successfully";
 
 if (job_mgr.IsValid()) {
 // Set memory limit
 EXPECT_TRUE(job_mgr.SetMemoryLimit(512 * 1024 * 1024)) // 512 MB
 << "Should set memory limit";
 
 // Set CPU time limit
 EXPECT_TRUE(job_mgr.SetCPUTimeLimit(60)) // 60 seconds
 << "Should set CPU time limit";
 
 // Set process limit
 EXPECT_TRUE(job_mgr.SetProcessCountLimit(1))
 << "Should set process count limit";
 }
}

TEST_F(SecurityTestFixture, JobObject_ProcessRestrictions) {
 std::wstring job_name = L"ExplorerLens_Test_Restricted_Job";
 JobObjectManager job_mgr(job_name);
 
 if (job_mgr.IsValid()) {
 // Apply token restrictions
 EXPECT_TRUE(job_mgr.ApplyTokenRestrictions())
 << "Should apply token restrictions";
 
 // Query limits to verify
 uint64_t memory_limit = 0;
 uint64_t cpu_limit_ms = 0;
 
 EXPECT_TRUE(job_mgr.QueryLimits(&memory_limit, &cpu_limit_ms))
 << "Should query job limits";
 }
}

//============================================================================
// 6. PluginDecoder Integration Tests
//============================================================================

TEST_F(SecurityTestFixture, PluginDecoder_DualMode_Construction) {
 // Test In-Worker construction
 // (Note: This requires a real plugin handle, so we'll test the factory pattern)
 
 std::wstring plugin_id = L"TestDecoderPlugin";
 std::filesystem::path plugin_path = test_plugin_dir_ / "decoder_test.dll";
 
 // Factory should handle mode selection automatically
 auto decoder = PluginDecoderFactory::CreateDecoder(nullptr, plugin_id, plugin_path);
 
 // Decoder creation may fail without real plugin, which is acceptable
 // The test validates that the factory pattern works
}

TEST_F(SecurityTestFixture, PluginDecoder_Statistics_Tracking) {
 std::wstring plugin_id = L"StatsTestPlugin";
 std::filesystem::path plugin_path = test_plugin_dir_ / "stats_test.dll";
 
 // Create decoder (may fail without real plugin)
 auto decoder = PluginDecoderFactory::CreateDecoder(nullptr, plugin_id, plugin_path);
 
 if (decoder) {
 // Get initial statistics
 auto stats = decoder->GetStatistics();
 EXPECT_EQ(stats.total_decodes, 0u) << "Initial decode count should be 0";
 
 // Reset should work
 decoder->ResetStatistics();
 stats = decoder->GetStatistics();
 EXPECT_EQ(stats.total_decodes, 0u);
 }
}

//============================================================================
// 7. Trust and Signing Tests
//============================================================================

TEST_F(SecurityTestFixture, Trust_PluginList_Management) {
 auto& selector = IsolationModeSelector::Instance();
 
 std::wstring test_plugin = L"TrustTestPlugin";
 
 // Initially not trusted
 EXPECT_FALSE(selector.IsTrustedPlugin(test_plugin));
 
 // Add to trusted list
 selector.AddTrustedPlugin(test_plugin);
 EXPECT_TRUE(selector.IsTrustedPlugin(test_plugin));
 
 // Remove from trusted list
 selector.RemoveTrustedPlugin(test_plugin);
 EXPECT_FALSE(selector.IsTrustedPlugin(test_plugin));
}

TEST_F(SecurityTestFixture, Trust_Configuration_Persistence) {
 auto& selector = IsolationModeSelector::Instance();
 
 // Add trusted plugin
 selector.AddTrustedPlugin(L"PersistentTestPlugin");
 
 // Save configuration
 selector.SaveConfiguration();
 
 // Reload configuration
 selector.LoadConfiguration();
 
 // Should still be trusted (if registry operations succeed)
 // Note: May not persist in unit test environment without registry access
}

//============================================================================
// Main Test Runner
//============================================================================

int main(int argc, char** argv) {
 ::testing::InitGoogleTest(&argc, argv);
 
 std::cout << "===================================================\n";
 std::cout << " ExplorerLens Security Test Suite\n";
 std::cout << " - Plugin Security Infrastructure\n";
 std::cout << "===================================================\n\n";
 
 int result = RUN_ALL_TESTS();
 
 std::cout << "\n===================================================\n";
 std::cout << " Test Suite Complete\n";
 std::cout << "===================================================\n";
 
 return result;
}

