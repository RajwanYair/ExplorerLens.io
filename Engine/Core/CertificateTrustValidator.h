#pragma once
// CertificateTrustValidator.h — Certificate Trust Validator
// Validates code-signing certificates for plugins and decode libraries,
// enforcing trust chains rooted in approved CAs.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Certificate validation result
enum class CertValidationResult : uint8_t {
 Valid = 0,
 Expired,
 Revoked,
 UntrustedRoot,
 InvalidSignature,
 SelfSigned,
 NotFound,
 COUNT
};

/// Trust level
enum class CertTrustLevel : uint8_t {
 Untrusted = 0,
 UserTrusted, // User explicitly trusted
 EnterpriseTrusted, // Enterprise CA trusted
 MicrosoftSigned, // Microsoft Authenticode
 ExplorerLensSigned, // Signed by ExplorerLens root
 COUNT
};

struct CertificateInfo {
 const wchar_t *subject = nullptr;
 const wchar_t *issuer = nullptr;
 const wchar_t *thumbprint = nullptr;
 uint64_t notBefore = 0;
 uint64_t notAfter = 0;
 CertValidationResult validation = CertValidationResult::NotFound;
 CertTrustLevel trustLevel = CertTrustLevel::Untrusted;
 bool isEV = false; // Extended Validation
};

class CertificateTrustValidator {
public:
 static constexpr size_t ValidationCount() {
 return static_cast<size_t>(CertValidationResult::COUNT);
 }
 static constexpr size_t TrustCount() {
 return static_cast<size_t>(CertTrustLevel::COUNT);
 }

 static const wchar_t *ValidationName(CertValidationResult r) {
 switch (r) {
 case CertValidationResult::Valid:
 return L"Valid";
 case CertValidationResult::Expired:
 return L"Expired";
 case CertValidationResult::Revoked:
 return L"Revoked";
 case CertValidationResult::UntrustedRoot:
 return L"Untrusted Root";
 case CertValidationResult::InvalidSignature:
 return L"Invalid Signature";
 case CertValidationResult::SelfSigned:
 return L"Self-Signed";
 case CertValidationResult::NotFound:
 return L"Not Found";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *TrustName(CertTrustLevel t) {
 switch (t) {
 case CertTrustLevel::Untrusted:
 return L"Untrusted";
 case CertTrustLevel::UserTrusted:
 return L"User Trusted";
 case CertTrustLevel::EnterpriseTrusted:
 return L"Enterprise Trusted";
 case CertTrustLevel::MicrosoftSigned:
 return L"Microsoft Signed";
 case CertTrustLevel::ExplorerLensSigned:
 return L"ExplorerLens Signed";
 default:
 return L"Unknown";
 }
 }

 /// Check if trust level is sufficient for plugin loading
 static bool IsSufficientTrust(CertTrustLevel level, CertTrustLevel minimum) {
 return static_cast<uint8_t>(level) >= static_cast<uint8_t>(minimum);
 }
};

} // namespace Engine
} // namespace ExplorerLens
