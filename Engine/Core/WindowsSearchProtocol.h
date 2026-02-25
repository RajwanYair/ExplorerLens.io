#pragma once
// Sprint 419: Windows Search Protocol Handler
// IFilter + ISearchProtocol implementation for Windows Desktop Search
// integration, enabling thumbnail metadata to appear in Windows Search results.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Search index field
enum class SearchIndexField : uint8_t {
  FileName = 0,
  FileExtension,
  Dimensions,
  ColorSpace_,
  CameraModel,
  DateTaken,
  GPSLocation,
  FileSize,
  FormatFamily_,
  ThumbnailHash,
  COUNT
};

/// Search protocol state
enum class SearchProtocolState : uint8_t {
  Unregistered = 0,
  Registered,
  Indexing,
  Idle,
  Error,
  COUNT
};

struct SearchIndexStats {
  uint64_t filesIndexed = 0;
  uint64_t fieldsStored = 0;
  double indexBuildTimeMs = 0.0;
  uint64_t indexSizeBytes = 0;
  SearchProtocolState state = SearchProtocolState::Unregistered;
};

class WindowsSearchProtocol {
public:
  static constexpr size_t FieldCount() {
    return static_cast<size_t>(SearchIndexField::COUNT);
  }
  static constexpr size_t StateCount() {
    return static_cast<size_t>(SearchProtocolState::COUNT);
  }

  static const wchar_t *FieldName(SearchIndexField f) {
    switch (f) {
    case SearchIndexField::FileName:
      return L"File Name";
    case SearchIndexField::FileExtension:
      return L"File Extension";
    case SearchIndexField::Dimensions:
      return L"Dimensions";
    case SearchIndexField::ColorSpace_:
      return L"Color Space";
    case SearchIndexField::CameraModel:
      return L"Camera Model";
    case SearchIndexField::DateTaken:
      return L"Date Taken";
    case SearchIndexField::GPSLocation:
      return L"GPS Location";
    case SearchIndexField::FileSize:
      return L"File Size";
    case SearchIndexField::FormatFamily_:
      return L"Format Family";
    case SearchIndexField::ThumbnailHash:
      return L"Thumbnail Hash";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *StateName(SearchProtocolState s) {
    switch (s) {
    case SearchProtocolState::Unregistered:
      return L"Unregistered";
    case SearchProtocolState::Registered:
      return L"Registered";
    case SearchProtocolState::Indexing:
      return L"Indexing";
    case SearchProtocolState::Idle:
      return L"Idle";
    case SearchProtocolState::Error:
      return L"Error";
    default:
      return L"Unknown";
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
