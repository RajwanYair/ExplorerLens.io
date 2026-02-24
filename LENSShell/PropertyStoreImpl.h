// PropertyStoreImpl.h — IPropertyStore / IPropertyStoreCapabilities for
// Explorer Details Pane ExplorerLens Engine v15.0.0 "Zenith" — Sprint 358-359
// Copyright (c) 2026 ExplorerLens Project
//
// Provides read-only metadata to Windows Explorer Details Pane for all
// supported file types. Exposes: image dimensions, color depth, codec name,
// item count (for archives), and format description.
#pragma once

#include <functional>
#include <propkey.h>
#include <propsys.h>
#include <propvarutil.h>
#include <string>
#include <vector>

#pragma comment(lib, "propsys.lib")

namespace ExplorerLens {

// ============================================================================
// Property descriptor for our exposed properties
// ============================================================================
struct PropertyDescriptor {
  PROPERTYKEY key;
  VARTYPE vt;
  std::wstring displayName;
};

// ============================================================================
// CLENSPropertyStore — Mixin implementation for IPropertyStore
// ============================================================================
// This class provides the IPropertyStore/IPropertyStoreCapabilities method
// implementations. CLENSShell inherits from this and delegates.
//
// Supported PROPERTYKEY values:
//   PKEY_Image_Dimensions       "1920 x 1080"
//   PKEY_Image_BitDepth         32
//   PKEY_Image_HorizontalSize   1920
//   PKEY_Image_VerticalSize     1080
//   PKEY_MIMEType               "application/zip"
//   PKEY_FileDescription        "ZIP Archive (5 images)"
//   PKEY_Kind                   "picture" / "document"
//   PKEY_ItemTypeText           "JPEG XL Image"
//   PKEY_Software               "ExplorerLens v15.0.0"
//   PKEY_Document_PageCount     5 (for multi-page/archive)

class CLENSPropertyStore {
public:
  CLENSPropertyStore() = default;
  virtual ~CLENSPropertyStore() = default;

  // ========================================================================
  // Initialize properties from file metadata
  // ========================================================================
  HRESULT InitializeProperties(const wchar_t *filePath, int lensType);

  // ========================================================================
  // IPropertyStore methods
  // ========================================================================
  HRESULT PropertyStore_GetCount(DWORD *cProps);
  HRESULT PropertyStore_GetAt(DWORD iProp, PROPERTYKEY *pkey);
  HRESULT PropertyStore_GetValue(REFPROPERTYKEY key, PROPVARIANT *pv);
  HRESULT PropertyStore_SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar);
  HRESULT PropertyStore_Commit();

  // ========================================================================
  // IPropertyStoreCapabilities methods
  // ========================================================================
  HRESULT PropertyStoreCapabilities_IsPropertyWritable(REFPROPERTYKEY key);

private:
  // Property storage
  struct StoredProperty {
    PROPERTYKEY key;
    PROPVARIANT value;

    StoredProperty() { PropVariantInit(&value); }
    ~StoredProperty() { PropVariantClear(&value); }

    // Non-copyable, movable
    StoredProperty(const StoredProperty &) = delete;
    StoredProperty &operator=(const StoredProperty &) = delete;
    StoredProperty(StoredProperty &&other) noexcept : key(other.key) {
      value = other.value;
      PropVariantInit(&other.value);
    }
    StoredProperty &operator=(StoredProperty &&other) noexcept {
      if (this != &other) {
        PropVariantClear(&value);
        key = other.key;
        value = other.value;
        PropVariantInit(&other.value);
      }
      return *this;
    }
  };

  std::vector<StoredProperty> m_properties;
  bool m_initialized = false;

  // Helpers
  void AddStringProperty(REFPROPERTYKEY key, const wchar_t *value);
  void AddUInt32Property(REFPROPERTYKEY key, UINT32 value);
  void AddStringVectorProperty(REFPROPERTYKEY key, const wchar_t *value);

  static std::wstring GetMimeTypeForExtension(const wchar_t *filePath);
  static std::wstring GetItemTypeText(int lensType);
  static std::wstring GetKindString(int lensType);
  static bool IsImageType(int lensType);
  static bool IsArchiveType(int lensType);

  const StoredProperty *FindProperty(REFPROPERTYKEY key) const;
};

} // namespace ExplorerLens
