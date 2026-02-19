// Sprint 154 — Plugin Trust Chain — GTest
#include <gtest/gtest.h>
#include "Plugin/PluginTrustChain.h"

using namespace DarkThumbs::Plugin;

TEST(PluginTrustChain, TrustLevelEnumOrder) {
    EXPECT_LT(static_cast<uint32_t>(PluginTrustLevel::Trusted),
              static_cast<uint32_t>(PluginTrustLevel::Blocked));
}

TEST(PluginTrustChain, CertificateChainEmptyDefault) {
    CertificateChain chain;
    EXPECT_TRUE(chain.entries.empty());
}

TEST(PluginTrustChain, SignatureBlockDefaultInvalid) {
    PluginSignatureBlock sig;
    EXPECT_FALSE(sig.isValid);
}

TEST(PluginTrustChain, RevocationEntryHasTimestamp) {
    RevocationEntry r;
    r.revokedAt = 12345678;
    EXPECT_GT(r.revokedAt, 0u);
}

TEST(PluginTrustChain, PublisherPolicyOpenDefault) {
    PublisherPolicy p;
    EXPECT_EQ(p.policy, PublisherPolicyType::Open);
}

TEST(PluginTrustChain, TrustValidationResultDefaultUntrusted) {
    TrustValidationResult r;
    EXPECT_EQ(r.level, PluginTrustLevel::Untrusted);
}

TEST(PluginTrustChain, ValidatorBlockedPluginReturnsBlocked) {
    PluginTrustChainValidator v;
    PluginSignatureBlock sig;
    sig.isValid = false;
    auto result = v.Validate(sig);
    EXPECT_NE(result.level, PluginTrustLevel::Trusted);
}

TEST(PluginTrustChain, TrustBadgeDisplayName) {
    TrustBadge b{ PluginTrustLevel::Trusted, "TrustBadge-Official" };
    EXPECT_FALSE(b.displayName.empty());
}

TEST(PluginTrustChain, CertEntryHasSubject) {
    CertificateEntry e;
    e.subject = "CN=DarkThumbs Official";
    EXPECT_FALSE(e.subject.empty());
}

TEST(PluginTrustChain, TrustLevelToStringNotEmpty) {
    auto s = ToString(PluginTrustLevel::Verified);
    EXPECT_FALSE(s.empty());
}

TEST(PluginTrustChain, ValidatorOfficial) {
    PluginTrustChainValidator v;
    PluginSignatureBlock sig;
    sig.isValid = true;
    sig.publisherCN = "DarkThumbs Official";
    auto result = v.Validate(sig);
    EXPECT_GE(static_cast<uint32_t>(result.level), static_cast<uint32_t>(PluginTrustLevel::Verified));
}

TEST(PluginTrustChain, RevocationListEmpty) {
    PluginTrustChainValidator v;
    EXPECT_TRUE(v.RevocationList().empty());
}

TEST(PluginTrustChain, BlockedPluginInRevocationList) {
    PluginTrustChainValidator v;
    RevocationEntry r;
    r.pluginId = "bad-plugin";
    v.AddRevocation(r);
    EXPECT_FALSE(v.RevocationList().empty());
}
