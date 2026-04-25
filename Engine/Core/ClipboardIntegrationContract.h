// ============================================================================
// ClipboardIntegrationContract.h -- S278 / ROADMAP v6.0 F7 shell clipboard
//
// Phase 3 contract for `explorerlens.copyThumbnail` / `explorerlens.copyMetadata`
// context-menu verbs.  Declares which clipboard formats (CF_DIBV5 / CF_PNG /
// CF_HDROP / CF_UNICODETEXT + custom JSON) are emitted, in what order of
// priority, and under what size cap.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class ClipboardIntegrationFormatId : uint16_t
{
    NONE               = 0,
    CLIP_DIBV5         = 1,   // CF_DIBV5 renamed to dodge winuser.h macro
    CLIP_PNG           = 2,   // registered "PNG" clipboard format
    CLIP_HDROP         = 3,   // CF_HDROP renamed
    CLIP_UNICODETEXT   = 4,   // CF_UNICODETEXT renamed
    EXPLORERLENS_JSON  = 5,   // "ExplorerLens.Metadata"
    CLIP_HTML          = 6,   // CF_HTML renamed
};

enum class ClipboardIntegrationOperation : uint8_t
{
    COPY_THUMBNAIL     = 0,
    COPY_METADATA_JSON = 1,
    COPY_SHA256_HEX    = 2,
    COPY_DECODER_INFO  = 3,
};

enum class ClipboardIntegrationStatus : uint8_t
{
    OK                 = 0,
    OPEN_FAILED        = 1,
    SET_DATA_FAILED    = 2,
    FORMAT_REGISTER_FAILED = 3,
    PAYLOAD_TOO_LARGE  = 4,
    BUDGET_EXCEEDED    = 5,
};

struct ClipboardIntegrationPlanRow
{
    ClipboardIntegrationOperation operation;
    ClipboardIntegrationFormatId  primaryFormat;
    ClipboardIntegrationFormatId  secondaryFormat;
    uint32_t                       sizeCapBytes;
};

inline constexpr ClipboardIntegrationPlanRow kClipboardIntegrationPlan[] = {
    { ClipboardIntegrationOperation::COPY_THUMBNAIL,
      ClipboardIntegrationFormatId::CLIP_DIBV5,
      ClipboardIntegrationFormatId::CLIP_PNG,
      16u * 1024u * 1024u },
    { ClipboardIntegrationOperation::COPY_METADATA_JSON,
      ClipboardIntegrationFormatId::EXPLORERLENS_JSON,
      ClipboardIntegrationFormatId::CLIP_UNICODETEXT,
      256u * 1024u },
    { ClipboardIntegrationOperation::COPY_SHA256_HEX,
      ClipboardIntegrationFormatId::CLIP_UNICODETEXT,
      ClipboardIntegrationFormatId::NONE,
      128u },
    { ClipboardIntegrationOperation::COPY_DECODER_INFO,
      ClipboardIntegrationFormatId::EXPLORERLENS_JSON,
      ClipboardIntegrationFormatId::CLIP_UNICODETEXT,
      8u * 1024u },
};
inline constexpr size_t kClipboardIntegrationPlanCount =
    sizeof(kClipboardIntegrationPlan) / sizeof(kClipboardIntegrationPlan[0]);

inline constexpr uint32_t kClipboardIntegrationMaxImageBytes = 32u * 1024u * 1024u;
inline constexpr uint32_t kClipboardIntegrationBudgetMs      = 50;

static_assert(kClipboardIntegrationPlanCount == 4,
              "Clipboard plan table must have 4 rows");
static_assert(std::is_trivially_copyable_v<ClipboardIntegrationPlanRow>,
              "ClipboardIntegrationPlanRow must be trivially copyable");

} // namespace ExplorerLens::Engine
