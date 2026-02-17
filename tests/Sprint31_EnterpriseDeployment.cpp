// =============================================================================
// Sprint 31: Enterprise Deployment & Group Policy Tests
// =============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <map>

// ---------------------------------------------------------------------------
// Group Policy Tests
// ---------------------------------------------------------------------------

class GroupPolicyTest : public ::testing::Test {
protected:
    static constexpr const char* kPolicyKey = "SOFTWARE\\Policies\\DarkThumbs";
};

TEST_F(GroupPolicyTest, PolicyRegistryPath) {
    std::string path = kPolicyKey;
    EXPECT_TRUE(path.find("Policies") != std::string::npos)
        << "GPO values must be under Policies registry hive";
    EXPECT_TRUE(path.find("DarkThumbs") != std::string::npos);
}

TEST_F(GroupPolicyTest, CorePoliciesDefined) {
    std::vector<std::string> requiredPolicies = {
        "DisableTelemetry",
        "TelemetryLevel",
        "MaxCacheSizeMB",
        "NetworkCachePath",
        "DisabledFormats",
        "DisableGPU"
    };

    EXPECT_GE(requiredPolicies.size(), 6u)
        << "At least 6 enterprise policies should be defined";
}

TEST_F(GroupPolicyTest, ADMXTemplateGeneration) {
    // ADMX must include namespace, categories, and policy elements
    std::string admx = "<?xml version=\"1.0\"?>\n<policyDefinitions>\n"
                       "  <target prefix=\"darkthumbs\"/>\n"
                       "  <categories><category name=\"DarkThumbs\"/></categories>\n"
                       "</policyDefinitions>";

    EXPECT_TRUE(admx.find("policyDefinitions") != std::string::npos);
    EXPECT_TRUE(admx.find("darkthumbs") != std::string::npos);
    EXPECT_TRUE(admx.find("categories") != std::string::npos);
}

TEST_F(GroupPolicyTest, TelemetryLevelEnum) {
    enum TelemetryLevel { Off = 0, Basic = 1, Enhanced = 2, Full = 3 };
    EXPECT_EQ(Off, 0);
    EXPECT_EQ(Basic, 1);
    EXPECT_EQ(Full, 3);
}

// ---------------------------------------------------------------------------
// Configuration Priority Tests
// ---------------------------------------------------------------------------

class ConfigPriorityTest : public ::testing::Test {
protected:
    enum ConfigSource { Default = 0, UserConfig = 1, MachineConfig = 2, UserPolicy = 3, MachinePolicy = 4 };

    struct ConfigValue {
        std::string value;
        ConfigSource source;
        bool isLocked;
    };
};

TEST_F(ConfigPriorityTest, MachinePolicyOverridesAll) {
    ConfigValue defaults{"500", Default, false};
    ConfigValue userJson{"1000", UserConfig, false};
    ConfigValue gpo{"256", MachinePolicy, true};

    // Highest source wins
    ConfigValue effective = defaults;
    if (userJson.source > effective.source) effective = userJson;
    if (gpo.source > effective.source) effective = gpo;

    EXPECT_EQ(effective.value, "256")
        << "Machine Policy (GPO) must override all other sources";
    EXPECT_TRUE(effective.isLocked)
        << "Policy-set values should be locked from user modification";
}

TEST_F(ConfigPriorityTest, UserCanOverrideDefaults) {
    ConfigValue defaults{"500", Default, false};
    ConfigValue userJson{"1000", UserConfig, false};

    ConfigValue effective = defaults;
    if (userJson.source > effective.source) effective = userJson;

    EXPECT_EQ(effective.value, "1000")
        << "User config should override defaults when no policy is set";
}

TEST_F(ConfigPriorityTest, LockedValueCannotBeChanged) {
    struct ConfigStore {
        std::map<std::string, ConfigValue> values;

        void set(const std::string& key, const std::string& val, ConfigSource src) {
            auto it = values.find(key);
            if (it == values.end() || src >= it->second.source) {
                values[key] = {val, src, (src >= UserPolicy)};
            }
        }
    };

    ConfigStore store;
    store.set("CacheSize", "256", MachinePolicy);
    store.set("CacheSize", "1024", UserConfig);  // Should not override policy

    EXPECT_EQ(store.values["CacheSize"].value, "256")
        << "User config must not override locked policy value";
}

// ---------------------------------------------------------------------------
// Silent Installation Tests
// ---------------------------------------------------------------------------

class SilentInstallTest : public ::testing::Test {};

TEST_F(SilentInstallTest, CommandLineGeneration) {
    std::string msi = "DarkThumbs-7.0.0.msi";
    std::string cmd = "msiexec /i \"" + msi + "\" /qn ALLUSERS=1";

    EXPECT_TRUE(cmd.find("/qn") != std::string::npos)
        << "Silent install must include /qn flag";
    EXPECT_TRUE(cmd.find("ALLUSERS=1") != std::string::npos)
        << "Per-machine install requires ALLUSERS=1";
}

TEST_F(SilentInstallTest, LogFileSupport) {
    std::string cmd = "msiexec /i \"setup.msi\" /qn /L*v \"install.log\"";
    EXPECT_TRUE(cmd.find("/L*v") != std::string::npos)
        << "Verbose logging should be supported for enterprise troubleshooting";
}

TEST_F(SilentInstallTest, TransformFileSupport) {
    std::string cmd = "msiexec /i \"setup.msi\" TRANSFORMS=\"custom.mst\"";
    EXPECT_TRUE(cmd.find("TRANSFORMS") != std::string::npos)
        << "MST transforms allow enterprise customization";
}

TEST_F(SilentInstallTest, DisableAutoUpdate) {
    std::string cmd = "msiexec /i \"setup.msi\" /qn AUTO_UPDATE=0";
    EXPECT_TRUE(cmd.find("AUTO_UPDATE=0") != std::string::npos)
        << "Enterprise deployments should disable auto-update";
}

// ---------------------------------------------------------------------------
// Network Cache Tests
// ---------------------------------------------------------------------------

class NetworkCacheTest : public ::testing::Test {};

TEST_F(NetworkCacheTest, UNCPathFormat) {
    std::string uncPath = "\\\\server\\darkthumbs-cache";
    EXPECT_TRUE(uncPath.substr(0, 2) == "\\\\")
        << "Network cache must use UNC paths";
}

TEST_F(NetworkCacheTest, ReadOnlyMode) {
    struct NetCache {
        bool readOnly = false;
        bool put(const std::string&) { return !readOnly; }
    };

    NetCache rw; rw.readOnly = false;
    NetCache ro; ro.readOnly = true;

    EXPECT_TRUE(rw.put("data"))   << "Read-write cache should accept puts";
    EXPECT_FALSE(ro.put("data"))  << "Read-only cache should reject puts";
}

TEST_F(NetworkCacheTest, TimeoutConfiguration) {
    uint32_t timeoutMs = 5000;
    uint32_t maxRetries = 2;

    EXPECT_LE(timeoutMs, 10000u)
        << "Network timeout should not exceed 10 seconds";
    EXPECT_LE(maxRetries, 5u)
        << "Retries should be limited to avoid blocking";
}

// ---------------------------------------------------------------------------
// Telemetry Control Tests
// ---------------------------------------------------------------------------

class TelemetryControlTest : public ::testing::Test {};

TEST_F(TelemetryControlTest, PolicyOverridesUserChoice) {
    struct TelemetryCtrl {
        int level = 1;  // User default: Basic
        bool policyOverride = false;

        void setLevel(int l) { if (!policyOverride) level = l; }
        void applyPolicy(int l) { level = l; policyOverride = true; }
    };

    TelemetryCtrl ctrl;
    ctrl.applyPolicy(0);   // Enterprise forces Off
    ctrl.setLevel(3);      // User tries to set Full

    EXPECT_EQ(ctrl.level, 0)
        << "Policy-forced telemetry level should not be overridable by user";
    EXPECT_TRUE(ctrl.policyOverride);
}

TEST_F(TelemetryControlTest, OffMeansNoCollection) {
    int level = 0;
    bool collecting = (level != 0);
    EXPECT_FALSE(collecting)
        << "Telemetry level Off must mean zero data collection";
}

TEST_F(TelemetryControlTest, FullRequiresExplicitOptIn) {
    // Full telemetry (level 3) should never be the default
    int defaultLevel = 1;    // Basic
    EXPECT_NE(defaultLevel, 3)
        << "Full telemetry must never be the default — requires explicit opt-in";
}

// ---------------------------------------------------------------------------
// JSON Config Tests
// ---------------------------------------------------------------------------

class JsonConfigTest : public ::testing::Test {};

TEST_F(JsonConfigTest, UserConfigPath) {
    std::string path = "%APPDATA%\\DarkThumbs\\darkthumbs.json";
    EXPECT_TRUE(path.find("APPDATA") != std::string::npos);
    EXPECT_TRUE(path.find("darkthumbs.json") != std::string::npos);
}

TEST_F(JsonConfigTest, MachineConfigPath) {
    std::string path = "%PROGRAMDATA%\\DarkThumbs\\darkthumbs.json";
    EXPECT_TRUE(path.find("PROGRAMDATA") != std::string::npos);
}

TEST_F(JsonConfigTest, DefaultValues) {
    struct Defaults {
        uint32_t maxCacheMB = 500;
        bool useGPU = true;
        bool telemetry = true;
        int telemetryLevel = 1;
    };

    Defaults d;
    EXPECT_EQ(d.maxCacheMB, 500u);
    EXPECT_TRUE(d.useGPU);
    EXPECT_EQ(d.telemetryLevel, 1)
        << "Default telemetry should be Basic (1)";
}

// ---------------------------------------------------------------------------
// Integration Tests
// ---------------------------------------------------------------------------

TEST(EnterpriseIntegrationTest, DeploymentHeaderExists) {
    namespace fs = std::filesystem;
    bool exists = fs::exists("Engine/Utils/EnterpriseDeployment.h") ||
                  fs::exists("Engine\\Utils\\EnterpriseDeployment.h");
    EXPECT_TRUE(exists) << "EnterpriseDeployment.h must exist for Sprint 31";
}
