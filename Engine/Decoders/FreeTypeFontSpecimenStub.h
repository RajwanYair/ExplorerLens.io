// ============================================================================
// FreeTypeFontSpecimenStub.h -- S266 / ROADMAP v6.0 L12 font thumbnails
//
// Evaluation contract for rendering font-file thumbnails (TTF/OTF/WOFF/WOFF2)
// as a specimen image ("The quick brown fox") via FreeType + HarfBuzz.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class FreeTypeFontSpecimenLayout : uint8_t
{
    SINGLE_LINE_LATIN       = 0,  // "Aa 123"
    PANGRAM_LATIN           = 1,  // "The quick brown fox..."
    GLYPH_GRID              = 2,  // 8x8 glyph grid
    SCRIPT_AUTODETECT       = 3,  // Latin / CJK / RTL based on cmap
};

enum class FreeTypeFontSpecimenStatus : uint8_t
{
    OK                      = 0,
    UNSUPPORTED_FORMAT      = 1,
    CORRUPT_TABLE           = 2,
    NO_RENDERABLE_GLYPHS    = 3,
    FACE_INIT_FAILED        = 4,
    BUDGET_EXCEEDED         = 5,
};

struct FreeTypeFontSpecimenOptions
{
    FreeTypeFontSpecimenLayout layout       = FreeTypeFontSpecimenLayout::PANGRAM_LATIN;
    uint32_t                   targetWidth  = 256;
    uint32_t                   targetHeight = 256;
    uint32_t                   pointSize    = 32;
    bool                       enableKerning = true;
    bool                       useHarfBuzz   = true;   // complex scripts
    bool                       subpixelAa    = true;
    uint32_t                   budgetMs      = 80;
};

struct FreeTypeFontSpecimenProbe
{
    FreeTypeFontSpecimenStatus status         = FreeTypeFontSpecimenStatus::OK;
    uint32_t                   glyphCount     = 0;
    uint32_t                   unitsPerEm     = 0;
    bool                       hasLatinCmap   = false;
    bool                       hasCjkCmap     = false;
    bool                       hasArabicCmap  = false;
    bool                       isColorFont    = false;   // COLR/SVG/sbix
    bool                       isVariableFont = false;   // fvar table present
};

inline constexpr uint32_t kFreeTypeFontSpecimenDefaultBudgetMs = 80;
inline constexpr uint32_t kFreeTypeFontSpecimenHardBudgetMs    = 300;
inline constexpr uint32_t kFreeTypeFontSpecimenMinPointSize    = 8;
inline constexpr uint32_t kFreeTypeFontSpecimenMaxPointSize    = 256;

static_assert(std::is_trivially_copyable_v<FreeTypeFontSpecimenOptions>,
              "FreeTypeFontSpecimenOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<FreeTypeFontSpecimenProbe>,
              "FreeTypeFontSpecimenProbe must be trivially copyable");

} // namespace ExplorerLens::Engine
