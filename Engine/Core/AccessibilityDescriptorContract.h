// ============================================================================
// AccessibilityDescriptorContract.h -- S279 / ROADMAP v6.0 F8 accessibility
//
// Phase 3 accessibility contract.  Every thumbnail emitted by LENSShell must
// carry a concise screen-reader description (UIA Name property) so blind
// users can navigate Explorer's grid view.  This header declares the
// descriptor fields, size caps, and localisation hook.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class AccessibilityDescriptorSource : uint8_t
{
    NONE                = 0,
    ALT_TEXT_EMBEDDED   = 1,   // PNG/SVG/PDF accessibility tag
    EXIF_IMAGE_DESC     = 2,   // EXIF 0x010E ImageDescription
    XMP_DC_DESCRIPTION  = 3,
    IPTC_CAPTION        = 4,
    AI_GENERATED        = 5,   // local AI captioner (opt-in)
    FILENAME_FALLBACK   = 6,
};

enum class AccessibilityDescriptorLocale : uint8_t
{
    AUTO     = 0,    // thread locale
    EN_US    = 1,
    EN_GB    = 2,
    FR_FR    = 3,
    DE_DE    = 4,
    ES_ES    = 5,
    PT_BR    = 6,
    JA_JP    = 7,
    ZH_CN    = 8,
    KO_KR    = 9,
    AR_SA    = 10,
    HE_IL    = 11,
};

enum class AccessibilityDescriptorStatus : uint8_t
{
    OK                   = 0,
    NO_SOURCE            = 1,   // nothing to describe
    TEXT_TOO_LONG        = 2,   // exceeded cap, truncated
    LOCALE_UNSUPPORTED   = 3,
    AI_DISABLED_BY_POLICY = 4,
    BUDGET_EXCEEDED      = 5,
};

struct AccessibilityDescriptorPolicy
{
    AccessibilityDescriptorLocale locale      = AccessibilityDescriptorLocale::AUTO;
    bool     allowAiGeneration                = false;   // opt-in only
    bool     includeDecoderHint               = true;    // "PNG image, 1024x768"
    bool     includeDimensions                = true;
    bool     includeFormatName                = true;
    uint32_t maxCharacters                    = 240;
    uint32_t budgetMs                         = 8;
};

struct AccessibilityDescriptorProbe
{
    AccessibilityDescriptorStatus status        = AccessibilityDescriptorStatus::OK;
    AccessibilityDescriptorSource chosenSource  = AccessibilityDescriptorSource::NONE;
    uint32_t                      emittedChars  = 0;
    bool                          wasTruncated  = false;
    bool                          usedAi        = false;
};

inline constexpr uint32_t kAccessibilityDescriptorHardMaxChars   = 1024;
inline constexpr uint32_t kAccessibilityDescriptorDefaultChars   = 240;
inline constexpr uint32_t kAccessibilityDescriptorDefaultBudgetMs = 8;

static_assert(std::is_trivially_copyable_v<AccessibilityDescriptorPolicy>,
              "AccessibilityDescriptorPolicy must be trivially copyable");
static_assert(std::is_trivially_copyable_v<AccessibilityDescriptorProbe>,
              "AccessibilityDescriptorProbe must be trivially copyable");
static_assert(kAccessibilityDescriptorDefaultChars <
              kAccessibilityDescriptorHardMaxChars,
              "default description length < hard max");

} // namespace ExplorerLens::Engine
