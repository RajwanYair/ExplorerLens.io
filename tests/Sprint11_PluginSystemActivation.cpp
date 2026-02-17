//==============================================================================
// DarkThumbs — Sprint 11 Tests: Plugin System Activation
// Tests feature flags, plugin state machine, discovery, IPC channel,
// lifecycle management, and sample plugin specifications.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Header under test
#include "../Engine/Plugin/PluginActivation.h"

using namespace DarkThumbs::Engine::Plugin;

//==============================================================================
// Plugin Feature Flag Tests
//==============================================================================

TEST(PluginFlags, DefaultDisabled)
{
    PluginFeatureFlags flags;
    EXPECT_FALSE(flags.enablePlugins);
    EXPECT_FALSE(flags.enableIPC);
    EXPECT_TRUE(flags.enableDiscovery);
}

TEST(PluginFlags, Production)
{
    auto flags = PluginFeatureFlags::Production();
    EXPECT_TRUE(flags.enablePlugins);
    EXPECT_TRUE(flags.enableIPC);
    EXPECT_TRUE(flags.enableSandbox);
    EXPECT_FALSE(flags.enableMarketplace);
    EXPECT_FALSE(flags.enableHotReload);
}

TEST(PluginFlags, AllEnabled)
{
    auto flags = PluginFeatureFlags::AllEnabled();
    EXPECT_EQ(flags.EnabledCount(), 6u);
}

TEST(PluginFlags, Disabled)
{
    auto flags = PluginFeatureFlags::Disabled();
    EXPECT_FALSE(flags.enablePlugins);
    EXPECT_EQ(flags.EnabledCount(), 2u);  // discovery + sandbox default true
}

TEST(PluginFlags, MaxPlugins)
{
    PluginFeatureFlags flags;
    EXPECT_EQ(flags.maxPlugins, 32u);
    EXPECT_EQ(flags.ipcTimeoutMs, 5000u);
}

//==============================================================================
// Plugin State Machine Tests
//==============================================================================

TEST(PluginState, StateNames)
{
    EXPECT_STREQ(PluginStateName(PluginState::Discovered), "Discovered");
    EXPECT_STREQ(PluginStateName(PluginState::Validated),  "Validated");
    EXPECT_STREQ(PluginStateName(PluginState::Loading),    "Loading");
    EXPECT_STREQ(PluginStateName(PluginState::Active),     "Active");
    EXPECT_STREQ(PluginStateName(PluginState::Suspended),  "Suspended");
    EXPECT_STREQ(PluginStateName(PluginState::Error),      "Error");
    EXPECT_STREQ(PluginStateName(PluginState::Unloaded),   "Unloaded");
}

TEST(PluginState, OperationalStates)
{
    EXPECT_TRUE(IsOperational(PluginState::Active));
    EXPECT_TRUE(IsOperational(PluginState::Suspended));
    EXPECT_FALSE(IsOperational(PluginState::Discovered));
    EXPECT_FALSE(IsOperational(PluginState::Error));
    EXPECT_FALSE(IsOperational(PluginState::Unloaded));
}

//==============================================================================
// Plugin Descriptor Tests
//==============================================================================

TEST(PluginDescriptor, DefaultState)
{
    PluginDescriptor p;
    EXPECT_EQ(p.state, PluginState::Discovered);
    EXPECT_FALSE(p.IsActive());
    EXPECT_FALSE(p.HasErrors());
}

TEST(PluginDescriptor, ErrorRate)
{
    PluginDescriptor p;
    p.decodeCount = 100;
    p.errorCount = 5;
    EXPECT_DOUBLE_EQ(p.ErrorRate(), 5.0);
}

TEST(PluginDescriptor, ErrorRateZeroDecodes)
{
    PluginDescriptor p;
    EXPECT_DOUBLE_EQ(p.ErrorRate(), 0.0);
}

TEST(PluginDescriptor, StatusBadge)
{
    PluginDescriptor p;
    p.state = PluginState::Active;
    EXPECT_EQ(p.StatusBadge(), "[Active]");
    p.state = PluginState::Error;
    EXPECT_EQ(p.StatusBadge(), "[Error]");
}

TEST(PluginDescriptor, FormatCount)
{
    PluginDescriptor p;
    p.supportedFormats = {".jpg", ".png", ".webp"};
    EXPECT_EQ(p.FormatCount(), 3u);
}

//==============================================================================
// Plugin Discovery Tests
//==============================================================================

TEST(PluginDiscovery, DefaultPaths)
{
    auto paths = PluginDiscovery::DefaultSearchPaths();
    EXPECT_EQ(paths.size(), 3u);
    // Should include LocalAppData path
    bool hasLocalAppData = false;
    for (auto& p : paths)
        if (p.find("LocalAppData") != std::string::npos) hasLocalAppData = true;
    EXPECT_TRUE(hasLocalAppData);
}

TEST(PluginDiscovery, ManifestFilename)
{
    EXPECT_EQ(PluginDiscovery::ManifestFilename(), "plugin.json");
}

TEST(PluginDiscovery, ScanEmpty)
{
    PluginDiscovery discovery;
    auto plugins = discovery.ScanDirectory("C:\\nonexistent");
    EXPECT_EQ(plugins.size(), 0u);
}

TEST(PluginDiscovery, AddAndScan)
{
    PluginDiscovery discovery;
    discovery.AddPlugin(SamplePluginSpec::MinimalPlugin());
    discovery.AddPlugin(SamplePluginSpec::RawEnhancedPlugin());
    EXPECT_EQ(discovery.PluginCount(), 2u);
    auto plugins = discovery.ScanDirectory(".");
    EXPECT_EQ(plugins.size(), 2u);
}

TEST(PluginDiscovery, Clear)
{
    PluginDiscovery discovery;
    discovery.AddPlugin(SamplePluginSpec::MinimalPlugin());
    discovery.Clear();
    EXPECT_EQ(discovery.PluginCount(), 0u);
}

//==============================================================================
// IPC Channel Tests
//==============================================================================

TEST(IPCChannel, MessageTypeNames)
{
    EXPECT_STREQ(IPCMessageTypeName(IPCMessageType::Ping), "Ping");
    EXPECT_STREQ(IPCMessageTypeName(IPCMessageType::DecodeRequest), "DecodeRequest");
    EXPECT_STREQ(IPCMessageTypeName(IPCMessageType::Shutdown), "Shutdown");
}

TEST(IPCChannel, DefaultPipeName)
{
    IPCChannel ch;
    EXPECT_NE(ch.PipeName().find("DarkThumbs"), std::string::npos);
}

TEST(IPCChannel, ConnectDisconnect)
{
    IPCChannel ch;
    EXPECT_FALSE(ch.IsConnected());
    EXPECT_TRUE(ch.Connect());
    EXPECT_TRUE(ch.IsConnected());
    ch.Disconnect();
    EXPECT_FALSE(ch.IsConnected());
}

TEST(IPCChannel, SendWhenDisconnected)
{
    IPCChannel ch;
    IPCMessage msg{IPCMessageType::Ping, "", 1, false};
    EXPECT_FALSE(ch.Send(msg));
}

TEST(IPCChannel, SendWhenConnected)
{
    IPCChannel ch;
    ch.Connect();
    IPCMessage msg{IPCMessageType::Ping, "", 1, false};
    EXPECT_TRUE(ch.Send(msg));
    EXPECT_EQ(ch.MessagesSent(), 1u);
}

TEST(IPCChannel, PingResponse)
{
    IPCChannel ch;
    ch.Connect();
    ch.Send({IPCMessageType::Ping, "", 1, false});
    auto response = ch.Receive();
    EXPECT_TRUE(response.isResponse);
    EXPECT_EQ(response.type, IPCMessageType::HealthCheck);
}

TEST(IPCChannel, DecodeRequestResponse)
{
    IPCChannel ch;
    ch.Connect();
    ch.Send({IPCMessageType::DecodeRequest, "test.custom", 2, false});
    auto response = ch.Receive();
    EXPECT_TRUE(response.isResponse);
    EXPECT_EQ(response.type, IPCMessageType::DecodeResult);
}

//==============================================================================
// Plugin Lifecycle Manager Tests
//==============================================================================

TEST(Lifecycle, DisabledByDefault)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Disabled());
    EXPECT_FALSE(mgr.IsEnabled());
}

TEST(Lifecycle, EnabledInProduction)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    EXPECT_TRUE(mgr.IsEnabled());
}

TEST(Lifecycle, RegisterPlugin)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    auto ok = mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    EXPECT_TRUE(ok);
    EXPECT_EQ(mgr.TotalPlugins(), 1u);
}

TEST(Lifecycle, RegisterWhenDisabled)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Disabled());
    auto ok = mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    EXPECT_FALSE(ok);
    EXPECT_EQ(mgr.TotalPlugins(), 0u);
}

TEST(Lifecycle, ActivatePlugin)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    auto ok = mgr.ActivatePlugin("com.darkthumbs.minimal-plugin");
    EXPECT_TRUE(ok);
    EXPECT_EQ(mgr.ActivePlugins(), 1u);
}

TEST(Lifecycle, SuspendPlugin)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    mgr.ActivatePlugin("com.darkthumbs.minimal-plugin");
    auto ok = mgr.SuspendPlugin("com.darkthumbs.minimal-plugin");
    EXPECT_TRUE(ok);
    EXPECT_EQ(mgr.ActivePlugins(), 0u);
    auto* p = mgr.GetPlugin("com.darkthumbs.minimal-plugin");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->state, PluginState::Suspended);
}

TEST(Lifecycle, UnloadPlugin)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    mgr.ActivatePlugin("com.darkthumbs.minimal-plugin");
    mgr.UnloadPlugin("com.darkthumbs.minimal-plugin");
    auto* p = mgr.GetPlugin("com.darkthumbs.minimal-plugin");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->state, PluginState::Unloaded);
}

TEST(Lifecycle, MaxPluginsLimit)
{
    PluginFeatureFlags flags = PluginFeatureFlags::Production();
    flags.maxPlugins = 2;
    PluginLifecycleManager mgr(flags);

    PluginDescriptor p1 = SamplePluginSpec::MinimalPlugin();
    PluginDescriptor p2 = SamplePluginSpec::RawEnhancedPlugin();
    PluginDescriptor p3;
    p3.id = "com.test.overflow";
    p3.name = "Overflow Plugin";

    EXPECT_TRUE(mgr.RegisterPlugin(p1));
    EXPECT_TRUE(mgr.RegisterPlugin(p2));
    EXPECT_FALSE(mgr.RegisterPlugin(p3));  // Over limit
    EXPECT_EQ(mgr.TotalPlugins(), 2u);
}

TEST(Lifecycle, GetActivePluginIds)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    mgr.RegisterPlugin(SamplePluginSpec::RawEnhancedPlugin());
    mgr.ActivatePlugin("com.darkthumbs.minimal-plugin");
    mgr.ActivatePlugin("com.darkthumbs.raw-enhanced");
    auto ids = mgr.GetActivePluginIds();
    EXPECT_EQ(ids.size(), 2u);
}

TEST(Lifecycle, StatusReport)
{
    PluginLifecycleManager mgr(PluginFeatureFlags::Production());
    mgr.RegisterPlugin(SamplePluginSpec::MinimalPlugin());
    mgr.ActivatePlugin("com.darkthumbs.minimal-plugin");
    auto report = mgr.StatusReport();
    EXPECT_NE(report.find("Plugin Status Report"), std::string::npos);
    EXPECT_NE(report.find("Minimal Plugin"), std::string::npos);
}

//==============================================================================
// Sample Plugin Spec Tests
//==============================================================================

TEST(SamplePlugin, MinimalPlugin)
{
    auto p = SamplePluginSpec::MinimalPlugin();
    EXPECT_EQ(p.id, "com.darkthumbs.minimal-plugin");
    EXPECT_EQ(p.version, "1.0.0");
    EXPECT_TRUE(p.isSigned);
    EXPECT_EQ(p.FormatCount(), 2u);
}

TEST(SamplePlugin, RawEnhanced)
{
    auto p = SamplePluginSpec::RawEnhancedPlugin();
    EXPECT_EQ(p.id, "com.darkthumbs.raw-enhanced");
    EXPECT_GE(p.FormatCount(), 5u);
    EXPECT_TRUE(p.isSigned);
}
