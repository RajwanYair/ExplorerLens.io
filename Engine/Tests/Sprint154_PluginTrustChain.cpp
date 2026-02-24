// Plugin Trust Chain — GTestShim
#include "GTestShim.h"
#include "Plugin/PluginTrustChain.h"

using namespace ExplorerLens::Plugin;

TEST(PluginTrustChain, TrustLevelEnumOrder) {
  EXPECT_LT(static_cast<uint32_t>(PluginTrustLevel::Trusted),
            static_cast<uint32_t>(PluginTrustLevel::Blocked));
}

TEST(PluginTrustChain, CertificateChainEmptyDefault) {
  CertificateChain chain;
  EXPECT_TRUE(chain.entries.empty());
}

TEST(PluginTrustChain, SignatureBlockDefaultEmptySignature) {
  PluginSignatureBlock sig;
  EXPECT_TRUE(sig.signatureHex.empty());
}

TEST(PluginTrustChain, RevocationEntryHasTimestamp) {
  RevocationEntry r;
  r.revokedAtEpoch = 12345678;
  EXPECT_GT(r.revokedAtEpoch, 0u);
}

TEST(PluginTrustChain, PublisherPolicyOpenDefault) {
  PublisherPolicy p;
  // Open policy: allowedPublishers is empty (open list)
  EXPECT_TRUE(p.allowedPublishers.empty());
}

TEST(PluginTrustChain, TrustValidationResultDefaultUntrusted) {
  TrustValidationResult r;
  EXPECT_EQ(r.level, PluginTrustLevel::Untrusted);
}

TEST(PluginTrustChain, ValidatorBlockedPluginReturnsNotTrusted) {
  PublisherPolicy policy = PublisherPolicy::Open();
  PluginTrustChainValidator v(policy);
  PluginSignatureBlock sig;
  // Empty signature — should be denied
  auto result = v.Validate(sig);
  EXPECT_NE(result.level, PluginTrustLevel::Trusted);
}

TEST(PluginTrustChain, TrustBadgeDisplayIconNotEmpty) {
  TrustBadge b = TrustBadge::For(PluginTrustLevel::Trusted);
  EXPECT_FALSE(b.iconId.empty());
}

TEST(PluginTrustChain, CertEntryHasSubject) {
  CertificateEntry e;
  e.subject = "CN=ExplorerLens Official";
  EXPECT_FALSE(e.subject.empty());
}

TEST(PluginTrustChain, TrustLevelToStringNotEmpty) {
  auto s = ToString(PluginTrustLevel::Verified);
  EXPECT_FALSE(s.empty());
}

TEST(PluginTrustChain, ValidatorOfficialPublisher) {
  PublisherPolicy policy = PublisherPolicy::ExplorerLensOfficial();
  PluginTrustChainValidator v(policy);
  PluginSignatureBlock sig;
  sig.publisherId = "ExplorerLens";
  CertificateEntry leaf;
  leaf.subject = "CN=ExplorerLens";
  leaf.thumbprint = "aabbcc";
  leaf.notAfterEpoch = 9999999999ULL;
  sig.chain.entries.push_back(leaf);
  sig.chain.isComplete = true;
  sig.algorithmId = "sha256WithRSAEncryption";
  sig.signatureHex = "deadbeef";
  auto result = v.Validate(sig);
  EXPECT_GE(static_cast<uint32_t>(result.level),
            static_cast<uint32_t>(PluginTrustLevel::Trusted));
}

TEST(PluginTrustChain, RevocationCheckBlocksRevoked) {
  PublisherPolicy policy;
  RevocationEntry rev;
  rev.thumbprint = "bad-cert-thumb";
  policy.revokedCerts.push_back(rev);
  EXPECT_TRUE(policy.IsCertRevoked("bad-cert-thumb"));
}

TEST(PluginTrustChain, BlockedPublisherNotAllowed) {
  PublisherPolicy policy;
  policy.blockedPublishers.push_back("bad-plugin");
  EXPECT_FALSE(policy.IsPublisherAllowed("bad-plugin"));
}
