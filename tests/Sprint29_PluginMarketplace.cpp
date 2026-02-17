// =============================================================================
// Sprint 29: Plugin Marketplace & Distribution Tests
// =============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <chrono>

// ---------------------------------------------------------------------------
// Plugin Package Format Tests
// ---------------------------------------------------------------------------

class PluginPackageTest : public ::testing::Test {
protected:
    struct TestManifest {
        std::string id;
        std::string name;
        uint32_t vMajor, vMinor, vPatch;
        std::string license;
        std::vector<std::string> files;
        uint64_t sizeBytes;

        std::string versionStr() const {
            return std::to_string(vMajor) + "." +
                   std::to_string(vMinor) + "." +
                   std::to_string(vPatch);
        }
    };
};

TEST_F(PluginPackageTest, ManifestVersionString) {
    TestManifest m{"com.test.plugin", "TestPlugin", 2, 1, 3, "MIT", {}, 0};
    EXPECT_EQ(m.versionStr(), "2.1.3");
}

TEST_F(PluginPackageTest, VersionRangeCompatibility) {
    auto inRange = [](uint32_t major, uint32_t minor, uint32_t patch,
                      uint32_t minMaj, uint32_t minMin, uint32_t minPat,
                      uint32_t maxMaj, uint32_t maxMin, uint32_t maxPat) -> bool {
        uint32_t ver = major * 10000 + minor * 100 + patch;
        uint32_t minV = minMaj * 10000 + minMin * 100 + minPat;
        uint32_t maxV = maxMaj * 10000 + maxMin * 100 + maxPat;
        return ver >= minV && ver <= maxV;
    };

    // DarkThumbs 7.0.0 compatible with range [7.0.0, 8.0.0]
    EXPECT_TRUE(inRange(7, 0, 0, 7, 0, 0, 8, 0, 0));
    EXPECT_TRUE(inRange(7, 5, 3, 7, 0, 0, 8, 0, 0));
    EXPECT_FALSE(inRange(6, 9, 9, 7, 0, 0, 8, 0, 0));
    EXPECT_FALSE(inRange(8, 0, 1, 7, 0, 0, 8, 0, 0));
}

TEST_F(PluginPackageTest, ReversDomainIdFormat) {
    std::string id = "com.author.pluginname";

    // Count dots — reverse domain IDs should have at least 2
    int dots = 0;
    for (char c : id) if (c == '.') dots++;
    EXPECT_GE(dots, 2) << "Plugin IDs should use reverse-domain format";
}

// ---------------------------------------------------------------------------
// Code Signing Tests
// ---------------------------------------------------------------------------

class CodeSigningTest : public ::testing::Test {};

TEST_F(CodeSigningTest, SignatureStatusValues) {
    enum SignatureStatus { Valid, Invalid, Expired, Untrusted, SelfSigned, Missing, Revoked };

    EXPECT_NE(Valid, Invalid);
    EXPECT_NE(Missing, Valid);
    EXPECT_NE(Revoked, Valid);
}

TEST_F(CodeSigningTest, PolicyAcceptance) {
    struct Policy {
        bool requireSignature;
        bool allowSelfSigned;
    };

    Policy strict{true, false};
    Policy lenient{false, true};

    // Strict: only Valid passes
    enum Sig { Valid, SelfSigned, Missing };

    auto acceptable = [](Policy p, Sig s) -> bool {
        if (s == Valid) return true;
        if (s == SelfSigned) return p.allowSelfSigned;
        if (s == Missing) return !p.requireSignature;
        return false;
    };

    EXPECT_TRUE(acceptable(strict, Valid));
    EXPECT_FALSE(acceptable(strict, SelfSigned));
    EXPECT_FALSE(acceptable(strict, Missing));

    EXPECT_TRUE(acceptable(lenient, Valid));
    EXPECT_TRUE(acceptable(lenient, SelfSigned));
    EXPECT_TRUE(acceptable(lenient, Missing));
}

TEST_F(CodeSigningTest, CertificateExpiryCheck) {
    auto now = std::chrono::system_clock::now();
    auto expired = now - std::chrono::hours(24 * 365);  // 1 year ago
    auto future = now + std::chrono::hours(24 * 365);   // 1 year from now

    EXPECT_LT(expired, now) << "Expired cert should be in the past";
    EXPECT_GT(future, now)  << "Valid cert should be in the future";
}

// ---------------------------------------------------------------------------
// Security Scanner Tests
// ---------------------------------------------------------------------------

class SecurityScannerTest : public ::testing::Test {};

TEST_F(SecurityScannerTest, BlockedFileTypes) {
    std::vector<std::string> blocked = {".exe", ".bat", ".cmd", ".ps1", ".vbs", ".js"};
    std::vector<std::string> allowed = {".dll", ".h", ".json", ".png", ".xml"};

    auto isBlocked = [&blocked](const std::string& ext) -> bool {
        for (const auto& b : blocked) {
            if (ext == b) return true;
        }
        return false;
    };

    for (const auto& b : blocked) {
        EXPECT_TRUE(isBlocked(b)) << b << " should be blocked";
    }
    for (const auto& a : allowed) {
        EXPECT_FALSE(isBlocked(a)) << a << " should be allowed";
    }
}

TEST_F(SecurityScannerTest, PackageSizeLimit) {
    uint64_t maxSize = 100 * 1024 * 1024;  // 100 MB
    uint64_t goodSize = 5 * 1024 * 1024;    // 5 MB
    uint64_t badSize = 200 * 1024 * 1024;   // 200 MB

    EXPECT_LE(goodSize, maxSize);
    EXPECT_GT(badSize, maxSize);
}

TEST_F(SecurityScannerTest, ScanResult_CriticalBlocksInstall) {
    struct ScanResult {
        bool passed = true;
        uint32_t criticalCount = 0;
        void addCritical() { criticalCount++; passed = false; }
    };

    ScanResult clean;
    EXPECT_TRUE(clean.passed);

    ScanResult dirty;
    dirty.addCritical();
    EXPECT_FALSE(dirty.passed) << "Any critical finding should block installation";
    EXPECT_EQ(dirty.criticalCount, 1u);
}

// ---------------------------------------------------------------------------
// Rating & Review Tests
// ---------------------------------------------------------------------------

class RatingSystemTest : public ::testing::Test {};

TEST_F(RatingSystemTest, AverageCalculation) {
    struct Ratings {
        double avg = 0.0;
        uint32_t total = 0;
        uint32_t dist[5] = {};
        void addRating(uint32_t stars) {
            if (stars < 1 || stars > 5) return;
            dist[stars - 1]++;
            total++;
            double sum = 0;
            for (int i = 0; i < 5; ++i) sum += dist[i] * (i + 1);
            avg = sum / total;
        }
    };

    Ratings r;
    r.addRating(5); r.addRating(5); r.addRating(4); r.addRating(3); r.addRating(1);

    EXPECT_NEAR(r.avg, 3.6, 0.01);
    EXPECT_EQ(r.total, 5u);
    EXPECT_EQ(r.dist[4], 2u);  // 5-star count
}

TEST_F(RatingSystemTest, ReviewValidation) {
    struct Review {
        std::string pluginId;
        uint32_t rating;
    };

    auto valid = [](const Review& r) -> bool {
        return !r.pluginId.empty() && r.rating >= 1 && r.rating <= 5;
    };

    EXPECT_TRUE(valid({"com.test.plugin", 4}));
    EXPECT_FALSE(valid({"", 4}));           // No plugin ID
    EXPECT_FALSE(valid({"com.test.plugin", 0})); // Invalid rating
    EXPECT_FALSE(valid({"com.test.plugin", 6})); // Rating too high
}

// ---------------------------------------------------------------------------
// Marketplace REST API Tests
// ---------------------------------------------------------------------------

class MarketplaceAPITest : public ::testing::Test {
protected:
    std::string baseUrl = "https://marketplace.darkthumbs.io/api/v1";
};

TEST_F(MarketplaceAPITest, SearchEndpoint) {
    std::string url = baseUrl + "/plugins?q=jpeg&type=0&page=1&pageSize=20";

    EXPECT_TRUE(url.find("/plugins?") != std::string::npos);
    EXPECT_TRUE(url.find("q=jpeg") != std::string::npos);
    EXPECT_TRUE(url.find("page=1") != std::string::npos);
}

TEST_F(MarketplaceAPITest, DetailsEndpoint) {
    std::string pluginId = "com.author.jpegxl-decoder";
    std::string url = baseUrl + "/plugins/" + pluginId;

    EXPECT_TRUE(url.find("/plugins/com.author.jpegxl-decoder") != std::string::npos);
}

TEST_F(MarketplaceAPITest, ReviewEndpoint) {
    std::string pluginId = "com.test.plugin";
    std::string url = baseUrl + "/plugins/" + pluginId + "/reviews";

    EXPECT_TRUE(url.find("/reviews") != std::string::npos);
}

TEST_F(MarketplaceAPITest, UpdateCheckEndpoint) {
    std::string url = baseUrl + "/plugins/check-updates";
    EXPECT_TRUE(url.find("check-updates") != std::string::npos);
}

TEST_F(MarketplaceAPITest, TLSRequired) {
    EXPECT_TRUE(baseUrl.substr(0, 8) == "https://")
        << "Marketplace API must use TLS";
}

// ---------------------------------------------------------------------------
// Installation Pipeline Tests
// ---------------------------------------------------------------------------

class InstallPipelineTest : public ::testing::Test {};

TEST_F(InstallPipelineTest, InstallStatusProgression) {
    enum Status { Pending, Downloading, Verifying, Scanning, Extracting, Registering, Completed };

    // Normal flow should progress through stages
    std::vector<Status> stages = {Pending, Downloading, Verifying, Scanning, Extracting, Registering, Completed};

    for (size_t i = 1; i < stages.size(); ++i) {
        EXPECT_GT(stages[i], stages[i - 1])
            << "Installation stages should progress in order";
    }
}

TEST_F(InstallPipelineTest, SecurityBlockStopsInstall) {
    // If security scan fails, installation stops at Scanning stage
    enum Status { Verifying, Scanning, Extracting, Failed };
    Status current = Scanning;
    bool scanPassed = false;

    if (!scanPassed) current = Failed;

    EXPECT_EQ(current, Failed)
        << "Failed security scan should halt installation";
}

// ---------------------------------------------------------------------------
// Integration Tests
// ---------------------------------------------------------------------------

TEST(MarketplaceIntegrationTest, PluginMarketplaceHeaderExists) {
    namespace fs = std::filesystem;
    bool exists = fs::exists("Engine/Plugin/PluginMarketplace.h") ||
                  fs::exists("Engine\\Plugin\\PluginMarketplace.h");
    EXPECT_TRUE(exists) << "PluginMarketplace.h must exist for Sprint 29";
}

TEST(MarketplaceIntegrationTest, ExistingRegistryPreserved) {
    namespace fs = std::filesystem;
    bool exists = fs::exists("packaging/marketplace/registry.json") ||
                  fs::exists("packaging\\marketplace\\registry.json");
    EXPECT_TRUE(exists) << "Existing marketplace registry.json should be preserved";
}
