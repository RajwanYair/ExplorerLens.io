#pragma once
// QuantumReadyHashPipeline.h — Quantum-Ready Hash Pipeline
// Future-proof content hashing using post-quantum algorithms (SHAKE-256,
// BLAKE3) alongside traditional SHA-256 for smooth migration path.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Hash algorithm selection
enum class QRHashAlgorithm : uint8_t {
 SHA256 = 0, // Traditional SHA-256
 SHA3_256, // SHA-3 (Keccak) 256-bit
 SHAKE256, // SHAKE-256 (variable output)
 BLAKE3, // BLAKE3 (fast, parallel)
 BLAKE2b, // BLAKE2b-256
 XXH3_128, // xxHash3 128-bit (non-crypto, fast)
 COUNT
};

/// Hash purpose classification
enum class HashPurpose : uint8_t {
 CacheKey = 0, // Fast, collision-resistant (XXH3/BLAKE3)
 ContentVerify, // File integrity (SHA-256/SHA-3)
 DigitalSignature, // Quantum-safe signing (SHAKE-256)
 Deduplication, // Content-based dedup (BLAKE3)
 COUNT
};

struct QRHashResult {
 uint8_t digest[64] = {}; // Max 512-bit hash
 uint32_t digestLen = 0; // Actual hash bytes
 QRHashAlgorithm algorithm = QRHashAlgorithm::SHA256;
 double computeMs = 0.0;
 uint64_t inputBytes = 0;
};

struct QRHashConfig {
 QRHashAlgorithm primary = QRHashAlgorithm::BLAKE3;
 QRHashAlgorithm fallback = QRHashAlgorithm::SHA256;
 bool dualHash = false; // Compute both for migration
 uint32_t outputBits = 256;
 bool hwAccelerated = true; // Use SHA-NI / AES-NI if available
 bool parallelChunked = true; // BLAKE3 parallel mode
};

class QuantumReadyHashPipeline {
public:
 static constexpr size_t AlgorithmCount() {
 return static_cast<size_t>(QRHashAlgorithm::COUNT);
 }
 static constexpr size_t PurposeCount() {
 return static_cast<size_t>(HashPurpose::COUNT);
 }

 static const wchar_t *AlgorithmName(QRHashAlgorithm a) {
 switch (a) {
 case QRHashAlgorithm::SHA256:
 return L"SHA-256";
 case QRHashAlgorithm::SHA3_256:
 return L"SHA-3-256";
 case QRHashAlgorithm::SHAKE256:
 return L"SHAKE-256";
 case QRHashAlgorithm::BLAKE3:
 return L"BLAKE3";
 case QRHashAlgorithm::BLAKE2b:
 return L"BLAKE2b";
 case QRHashAlgorithm::XXH3_128:
 return L"XXH3-128";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *PurposeName(HashPurpose p) {
 switch (p) {
 case HashPurpose::CacheKey:
 return L"Cache Key";
 case HashPurpose::ContentVerify:
 return L"Content Verification";
 case HashPurpose::DigitalSignature:
 return L"Digital Signature";
 case HashPurpose::Deduplication:
 return L"Deduplication";
 default:
 return L"Unknown";
 }
 }

 /// Recommend algorithm for purpose
 static QRHashAlgorithm RecommendAlgorithm(HashPurpose purpose) {
 switch (purpose) {
 case HashPurpose::CacheKey:
 return QRHashAlgorithm::XXH3_128;
 case HashPurpose::ContentVerify:
 return QRHashAlgorithm::BLAKE3;
 case HashPurpose::DigitalSignature:
 return QRHashAlgorithm::SHAKE256;
 case HashPurpose::Deduplication:
 return QRHashAlgorithm::BLAKE3;
 default:
 return QRHashAlgorithm::SHA256;
 }
 }

 /// Check if algorithm is quantum-resistant
 static bool IsQuantumSafe(QRHashAlgorithm algo) {
 // SHA-3 family and BLAKE3 with 256-bit output provide
 // 128-bit quantum security (Grover's algorithm halves security)
 switch (algo) {
 case QRHashAlgorithm::SHA3_256:
 case QRHashAlgorithm::SHAKE256:
 case QRHashAlgorithm::BLAKE3:
 case QRHashAlgorithm::BLAKE2b:
 return true;
 default:
 return false;
 }
 }
};

} // namespace Engine
} // namespace ExplorerLens
