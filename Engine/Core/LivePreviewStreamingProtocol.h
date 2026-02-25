#pragma once
// Sprint 448: Live Preview Streaming Protocol
// Real-time thumbnail streaming for remote/cloud files via
// progressive decode with adaptive quality based on bandwidth.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Stream quality tier
enum class StreamQuality : uint8_t {
  Placeholder = 0, // 1x1 color placeholder
  Icon_32,         // 32x32 icon-quality
  Thumbnail_128,   // 128x128 standard thumbnail
  Preview_512,     // 512x512 high-quality preview
  Full,            // Full resolution
  COUNT
};

/// Streaming protocol
enum class StreamProtocol : uint8_t {
  DirectRead = 0, // Local file read (baseline)
  HTTP_Range,     // HTTP range requests
  WebSocket,      // WebSocket streaming
  gRPC_Stream,    // gRPC bidirectional stream
  SMB_Direct,     // SMB Direct (RDMA)
  COUNT
};

struct StreamingStats {
  StreamQuality currentQuality = StreamQuality::Placeholder;
  StreamProtocol protocol = StreamProtocol::DirectRead;
  uint64_t bytesReceived = 0;
  uint64_t bytesTotal = 0;
  double bandwidthMBps = 0.0;
  double latencyMs = 0.0;
  uint32_t qualityUpgrades = 0;
  bool isComplete = false;
};

struct LivePreviewConfig {
  bool enabled = true;
  StreamProtocol preferredProto = StreamProtocol::HTTP_Range;
  StreamQuality initialQuality = StreamQuality::Thumbnail_128;
  StreamQuality maxQuality = StreamQuality::Preview_512;
  uint32_t timeoutMs = 10000;
  uint32_t maxConcurrent = 4;
  bool progressiveDecode = true;
  bool adaptiveQuality = true;
  double minBandwidthMBps = 0.5;
};

class LivePreviewStreamingProtocol {
public:
  static constexpr size_t QualityCount() {
    return static_cast<size_t>(StreamQuality::COUNT);
  }
  static constexpr size_t ProtocolCount() {
    return static_cast<size_t>(StreamProtocol::COUNT);
  }

  static const wchar_t *QualityName(StreamQuality q) {
    switch (q) {
    case StreamQuality::Placeholder:
      return L"Placeholder";
    case StreamQuality::Icon_32:
      return L"Icon 32x32";
    case StreamQuality::Thumbnail_128:
      return L"Thumbnail 128x128";
    case StreamQuality::Preview_512:
      return L"Preview 512x512";
    case StreamQuality::Full:
      return L"Full Resolution";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *ProtocolName(StreamProtocol p) {
    switch (p) {
    case StreamProtocol::DirectRead:
      return L"Direct Read";
    case StreamProtocol::HTTP_Range:
      return L"HTTP Range";
    case StreamProtocol::WebSocket:
      return L"WebSocket";
    case StreamProtocol::gRPC_Stream:
      return L"gRPC Stream";
    case StreamProtocol::SMB_Direct:
      return L"SMB Direct (RDMA)";
    default:
      return L"Unknown";
    }
  }

  /// Approximate pixel dimensions for quality tier
  static uint32_t QualityPixels(StreamQuality q) {
    switch (q) {
    case StreamQuality::Placeholder:
      return 1;
    case StreamQuality::Icon_32:
      return 32;
    case StreamQuality::Thumbnail_128:
      return 128;
    case StreamQuality::Preview_512:
      return 512;
    case StreamQuality::Full:
      return 0; // Variable
    default:
      return 128;
    }
  }

  /// Check if bandwidth is sufficient for quality
  static bool BandwidthSufficient(double bandwidthMBps, StreamQuality target) {
    switch (target) {
    case StreamQuality::Placeholder:
      return bandwidthMBps >= 0.01;
    case StreamQuality::Icon_32:
      return bandwidthMBps >= 0.05;
    case StreamQuality::Thumbnail_128:
      return bandwidthMBps >= 0.5;
    case StreamQuality::Preview_512:
      return bandwidthMBps >= 2.0;
    case StreamQuality::Full:
      return bandwidthMBps >= 10.0;
    default:
      return false;
    }
  }

  /// Select quality based on bandwidth
  static StreamQuality SelectQuality(double bandwidthMBps) {
    if (bandwidthMBps >= 10.0)
      return StreamQuality::Full;
    if (bandwidthMBps >= 2.0)
      return StreamQuality::Preview_512;
    if (bandwidthMBps >= 0.5)
      return StreamQuality::Thumbnail_128;
    if (bandwidthMBps >= 0.05)
      return StreamQuality::Icon_32;
    return StreamQuality::Placeholder;
  }
};

} // namespace Engine
} // namespace ExplorerLens
