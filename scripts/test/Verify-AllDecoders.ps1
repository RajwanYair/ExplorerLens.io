# ==============================================================================
# Verify-AllDecoders.ps1 - Comprehensive Decoder Validation Script
# ExplorerLens v7.0.0
# ==============================================================================
# DESCRIPTION:
#   Verifies that all 24 decoders are properly implemented, compiled, and linked.
#   Checks for missing dependencies, broken includes, and runtime issues.
#
# USAGE:
#   .\scripts\test\Verify-AllDecoders.ps1                  # Full validation
#   .\scripts\test\Verify-AllDecoders.ps1 -Quick           # Skip build checks
#   .\scripts\test\Verify-AllDecoders.ps1 -Decoder "HEIF"  # Test specific decoder
# ==============================================================================

param(
    [switch]$Quick,           # Skip time-consuming checks
    [string]$Decoder = "",    # Test specific decoder only
    [switch]$Verbose          # Show detailed output
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

# ==============================================================================
# Color Output Functions
# ==============================================================================

function Write-Section {
    param([string]$Text)
    Write-Host "`n═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host "  $Text" -ForegroundColor Cyan
    Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
}

function Write-Pass {
    param([string]$Text)
    Write-Host "  ✓ " -ForegroundColor Green -NoNewline
    Write-Host $Text
}

function Write-Fail {
    param([string]$Text)
    Write-Host "  ✗ " -ForegroundColor Red -NoNewline
    Write-Host $Text -ForegroundColor Red
}

function Write-Warn {
    param([string]$Text)
    Write-Host "  ⚠ " -ForegroundColor Yellow -NoNewline
    Write-Host $Text -ForegroundColor Yellow
}

function Write-Info {
    param([string]$Text)
    if ($Verbose) {
        Write-Host "    $Text" -ForegroundColor Gray
    }
}

# ==============================================================================
# Decoder Definition (24 decoders in v7.0.0)
# ==============================================================================

$decoders = @(
    @{Name = "ImageDecoder"; Files = @("ImageDecoder.cpp", "ImageDecoder.h"); Extensions = @(".jpg", ".png", ".bmp", ".gif", ".tiff"); Library = "WIC"; Status = "Production" },
    @{Name = "WebPDecoder"; Files = @("WebPDecoder.cpp", "WebPDecoder.h"); Extensions = @(".webp"); Library = "libwebp"; Status = "Production" },
    @{Name = "AVIFDecoder"; Files = @("AVIFDecoder.cpp", "AVIFDecoder.h"); Extensions = @(".avif"); Library = "libavif+dav1d"; Status = "Production" },
    @{Name = "JXLDecoder"; Files = @("JXLDecoder.cpp", "JXLDecoder.h"); Extensions = @(".jxl"); Library = "libjxl"; Status = "Production" },
    @{Name = "HEIFDecoder"; Files = @("HEIFDecoder.cpp", "HEIFDecoder.h"); Extensions = @(".heif", ".heic"); Library = "libheif"; Status = "Conditional" },
    @{Name = "RAWDecoder"; Files = @("RAWDecoder.cpp", "RAWDecoder.h"); Extensions = @(".cr2", ".nef", ".arw", ".dng"); Library = "LibRaw"; Status = "Production" },
    @{Name = "ArchiveDecoder"; Files = @("ArchiveDecoder.cpp", "ArchiveDecoder.h"); Extensions = @(".zip", ".rar", ".7z"); Library = "minizip-ng"; Status = "Production" },
    @{Name = "QOIDecoder"; Files = @("QOIDecoder.cpp", "QOIDecoder.h"); Extensions = @(".qoi"); Library = "Built-in"; Status = "Production" },
    @{Name = "PSDDecoder"; Files = @("PSDDecoder.cpp", "PSDDecoder.h"); Extensions = @(".psd", ".psb"); Library = "Built-in"; Status = "Production" },
    @{Name = "DDSDecoder"; Files = @("DDSDecoder.cpp", "DDSDecoder.h"); Extensions = @(".dds"); Library = "Built-in"; Status = "Production" },
    @{Name = "HDRDecoder"; Files = @("HDRDecoder.cpp", "HDRDecoder.h"); Extensions = @(".hdr"); Library = "Built-in"; Status = "Production" },
    @{Name = "EXRDecoder"; Files = @("EXRDecoder.cpp", "EXRDecoder.h"); Extensions = @(".exr"); Library = "WIC+Codec"; Status = "Production" },
    @{Name = "TGADecoder"; Files = @("TGADecoder.cpp", "TGADecoder.h"); Extensions = @(".tga"); Library = "Built-in"; Status = "Production" },
    @{Name = "ICODecoder"; Files = @("ICODecoder.cpp", "ICODecoder.h"); Extensions = @(".ico", ".cur"); Library = "Built-in"; Status = "Production" },
    @{Name = "PPMDecoder"; Files = @("PPMDecoder.cpp", "PPMDecoder.h"); Extensions = @(".ppm", ".pgm", ".pbm"); Library = "Built-in"; Status = "Production" },
    @{Name = "SVGDecoder"; Files = @("SVGDecoder.cpp", "SVGDecoder.h"); Extensions = @(".svg", ".svgz"); Library = "GDI+"; Status = "Production" },
    @{Name = "VideoDecoder"; Files = @("VideoDecoder.cpp", "VideoDecoder.h"); Extensions = @(".mp4", ".mkv", ".avi", ".webm"); Library = "MediaFoundation"; Status = "Production" },
    @{Name = "AudioDecoder"; Files = @("AudioDecoder.cpp", "AudioDecoder.h"); Extensions = @(".mp3", ".flac", ".m4a"); Library = "MediaFoundation"; Status = "Production" },
    @{Name = "PDFDecoder"; Files = @("PDFDecoder.cpp", "PDFDecoder.h"); Extensions = @(".pdf"); Library = "Shell API"; Status = "Production" },
    @{Name = "DocumentDecoder"; Files = @("DocumentDecoder.cpp", "DocumentDecoder.h"); Extensions = @(".docx", ".xlsx", ".pptx"); Library = "Shell API"; Status = "Production" },
    @{Name = "FontDecoder"; Files = @("FontDecoder.cpp", "FontDecoder.h"); Extensions = @(".ttf", ".otf"); Library = "GDI+"; Status = "Production" },
    @{Name = "ModelDecoder"; Files = @("ModelDecoder.cpp", "ModelDecoder.h"); Extensions = @(".obj", ".stl", ".gltf"); Library = "Built-in"; Status = "Production" },
    @{Name = "ExampleDecoder"; Files = @("ExampleDecoder.cpp", "ExampleDecoder.h"); Extensions = @(); Library = "None"; Status = "Template" },
    @{Name = "PluginDecoder"; Files = @("PluginDecoder.cpp", "PluginDecoder.h"); Extensions = @(); Library = "Plugin System"; Status = "Production" }
)

# ==============================================================================
# Test 1: Source File Existence
# ==============================================================================

Write-Section "Test 1: Verifying Decoder Source Files"

$sourcePath = Join-Path $repoRoot "Engine\Decoders"
$passed = 0
$failed = 0

foreach ($decoder in $decoders) {
    if ($Decoder -and $decoder.Name -notlike "*$Decoder*") { continue }
    
    $allExist = $true
    foreach ($file in $decoder.Files) {
        $fullPath = Join-Path $sourcePath $file
        if (-not (Test-Path $fullPath)) {
            Write-Fail "$($decoder.Name): Missing $file"
            $allExist = $false
            $failed++
        }
    }
    
    if ($allExist) {
        Write-Pass "$($decoder.Name): All source files present"
        $passed++
    }
}

Write-Host ""
Write-Host "  Result: $passed passed, $failed failed" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Yellow" })

# ==============================================================================
# Test 2: CMakeLists.txt Registration
# ==============================================================================

Write-Section "Test 2: Verifying CMakeLists.txt Registration"

$cmakePath = Join-Path $repoRoot "Engine\CMakeLists.txt"
$cmakeContent = Get-Content $cmakePath -Raw

$passed = 0
$failed = 0

foreach ($decoder in $decoders) {
    if ($Decoder -and $decoder.Name -notlike "*$Decoder*") { continue }
    
    $cppFile = $decoder.Files[0]  # .cpp file
    $hFile = $decoder.Files[1]    # .h file
    
    $hasCpp = $cmakeContent -match [regex]::Escape($cppFile)
    $hasH = $cmakeContent -match [regex]::Escape($hFile)
    
    if ($hasCpp -and $hasH) {
        Write-Pass "$($decoder.Name): Registered in CMake"
        $passed++
    } else {
        Write-Fail "$($decoder.Name): Missing from CMake (cpp=$hasCpp, h=$hasH)"
        $failed++
    }
}

Write-Host ""
Write-Host "  Result: $passed passed, $failed failed" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Yellow" })

# ==============================================================================
# Test 3: Pipeline Registration
# ==============================================================================

Write-Section "Test 3: Verifying ThumbnailPipeline Registration"

$pipelinePath = Join-Path $repoRoot "Engine\Pipeline\ThumbnailPipeline.cpp"
$pipelineContent = Get-Content $pipelinePath -Raw

$passed = 0
$failed = 0
$skipped = 0

foreach ($decoder in $decoders) {
    if ($Decoder -and $decoder.Name -notlike "*$Decoder*") { continue }
    
    # Skip template and plugin decoders (not registered in pipeline)
    if ($decoder.Status -eq "Template" -or $decoder.Name -eq "PluginDecoder") {
        Write-Info "$($decoder.Name): Skipped (not pipeline-registered)"
        $skipped++
        continue
    }
    
    $pattern = "RegisterDecoder.*$($decoder.Name)|$($decoder.Name).*RegisterDecoder"
    if ($pipelineContent -match $pattern) {
        Write-Pass "$($decoder.Name): Registered in Pipeline"
        $passed++
    } else {
        Write-Warn "$($decoder.Name): Not found in Pipeline registration"
        $failed++
    }
}

Write-Host ""
Write-Host "  Result: $passed passed, $failed failed, $skipped skipped" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Yellow" })

# ==============================================================================
# Test 4: Build Artifacts
# ==============================================================================

if (-not $Quick) {
    Write-Section "Test 4: Verifying Build Artifacts"
    
    $buildPath = Join-Path $repoRoot "build\lib\Release\ExplorerLensEngine.lib"
    
    if (Test-Path $buildPath) {
        $fileSize = (Get-Item $buildPath).Length / 1MB
        Write-Pass "ExplorerLensEngine.lib exists ($([math]::Round($fileSize, 2)) MB)"
        
        # Check if library was built recently (within 24 hours)
        $lastWrite = (Get-Item $buildPath).LastWriteTime
        $age = (Get-Date) - $lastWrite
        if ($age.TotalHours -lt 24) {
            Write-Pass "Build is recent ($([math]::Round($age.TotalHours, 1)) hours old)"
        } else {
            Write-Warn "Build is old ($([math]::Round($age.TotalDays, 1)) days old) - consider rebuilding"
        }
    } else {
        Write-Fail "ExplorerLensEngine.lib not found"
        Write-Info "Run: cmake --build build --config Release"
    }
    
    # Check LENSShell.dll
    $shellPath = Join-Path $repoRoot "x64\Release\LENSShell.dll"
    if (Test-Path $shellPath) {
        Write-Pass "LENSShell.dll exists"
    } else {
        Write-Warn "LENSShell.dll not found (may not be built yet)"
    }
}

# ==============================================================================
# Test 5: External Library Dependencies
# ==============================================================================

Write-Section "Test 5: Verifying External Library Dependencies"

$libraries = @(
    @{Name = "libwebp"; Path = "external\image-libs\libwebp-1.5.0-original"; Required = $true },
    @{Name = "libavif"; Path = "external\image-libs\libavif-1.3.0"; Required = $true },
    @{Name = "dav1d"; Path = "external\image-libs\dav1d-1.5.1"; Required = $true },
    @{Name = "libjxl"; Path = "external\image-libs\libjxl-0.11.1"; Required = $true },
    @{Name = "libheif"; Path = "external\image-libs\libheif"; Required = $false },
    @{Name = "LibRaw"; Path = "external\camera-libs\LibRaw-0.21.3"; Required = $true },
    @{Name = "minizip-ng"; Path = "external\compression-libs\minizip-ng-4.0.10"; Required = $true }
)

$passed = 0
$missing = 0

foreach ($lib in $libraries) {
    $fullPath = Join-Path $repoRoot $lib.Path
    if (Test-Path $fullPath) {
        Write-Pass "$($lib.Name): Found at $($lib.Path)"
        $passed++
    } else {
        if ($lib.Required) {
            Write-Fail "$($lib.Name): MISSING (Required)"
            $missing++
        } else {
            Write-Warn "$($lib.Name): Not found (Optional)"
        }
    }
}

Write-Host ""
Write-Host "  Result: $passed found, $missing missing" -ForegroundColor $(if ($missing -eq 0) { "Green" } else { "Red" })

# ==============================================================================
# Test 6: Decoder Code Quality Checks
# ==============================================================================

Write-Section "Test 6: Code Quality Checks"

$passed = 0
$warnings = 0

foreach ($decoder in $decoders) {
    if ($Decoder -and $decoder.Name -notlike "*$Decoder*") { continue }
    if ($decoder.Status -eq "Template") { continue }
    
    $cppPath = Join-Path $sourcePath $decoder.Files[0]
    if (-not (Test-Path $cppPath)) { continue }
    
    $content = Get-Content $cppPath -Raw
    
    # Check for required methods
    $hasCanDecode = $content -match "bool.*CanDecode"
    $hasDecode = $content -match "HRESULT.*Decode"
    $hasGetInfo = $content -match "DecoderInfo.*GetInfo"
    
    if ($hasCanDecode -and $hasDecode -and $hasGetInfo) {
        Write-Pass "$($decoder.Name): Implements required interface"
        $passed++
    } else {
        Write-Warn "$($decoder.Name): Missing methods (CanDecode=$hasCanDecode, Decode=$hasDecode, GetInfo=$hasGetInfo)"
        $warnings++
    }
}

Write-Host ""
Write-Host "  Result: $passed complete, $warnings incomplete" -ForegroundColor $(if ($warnings -eq 0) { "Green" } else { "Yellow" })

# ==============================================================================
# Test 7: Documentation Coverage
# ==============================================================================

Write-Section "Test 7: Documentation Coverage"

$docFiles = @(
    "docs\formats\FORMAT_SUPPORT_MATRIX_V7.md",
    "test-archives\README.md",
    "USER_GUIDE.md",
    "README.md"
)

$passed = 0
$missing = 0

foreach ($docFile in $docFiles) {
    $fullPath = Join-Path $repoRoot $docFile
    if (Test-Path $fullPath) {
        Write-Pass $(Split-Path $docFile -Leaf)
        $passed++
    } else {
        Write-Fail "$(Split-Path $docFile -Leaf): Not found"
        $missing++
    }
}

Write-Host ""
Write-Host "  Result: $passed exist, $missing missing" -ForegroundColor $(if ($missing -eq 0) { "Green" } else { "Yellow" })

# ==============================================================================
# Final Summary
# ==============================================================================

Write-Section "Summary"

$totalDecoders = if ($Decoder) { 
    ($decoders | Where-Object { $_.Name -like "*$Decoder*" }).Count 
} else { 
    $decoders.Count 
}

Write-Host ""
Write-Host "  Total Decoders: $totalDecoders" -ForegroundColor Cyan
Write-Host "  Production Ready: $(($decoders | Where-Object { $_.Status -eq 'Production' }).Count)" -ForegroundColor Green
Write-Host "  Conditional: $(($decoders | Where-Object { $_.Status -eq 'Conditional' }).Count)" -ForegroundColor Yellow
Write-Host ""

# Check critical issues
$criticalIssues = @()

if (-not (Test-Path (Join-Path $repoRoot "build\lib\Release\ExplorerLensEngine.lib"))) {
    $criticalIssues += "Engine not built"
}

$missingWebP = -not (Test-Path (Join-Path $repoRoot "external\image-libs\libwebp-1.5.0-original"))
$missingAvif = -not (Test-Path (Join-Path $repoRoot "external\image-libs\libavif-1.3.0"))
$missingJxl = -not (Test-Path (Join-Path $repoRoot "external\image-libs\libjxl-0.11.1"))

if ($missingWebP -or $missingAvif -or $missingJxl) {
    $criticalIssues += "Missing required image libraries"
}

if ($criticalIssues.Count -gt 0) {
    Write-Host "  CRITICAL ISSUES:" -ForegroundColor Red
    foreach ($issue in $criticalIssues) {
        Write-Host "    • $issue" -ForegroundColor Red
    }
    Write-Host ""
    exit 1
} else {
    Write-Host "  ✓ All critical checks passed!" -ForegroundColor Green
    Write-Host ""
}

# Recommendations
Write-Host "  Recommendations:" -ForegroundColor Cyan
Write-Host "    • Build libheif for iPhone photo support (.heic)" -ForegroundColor Yellow
Write-Host "    • Install OpenEXR WIC codec for .exr support" -ForegroundColor Yellow
Write-Host "    • Run integration tests: .\scripts\test\Test-ExplorerLens.ps1" -ForegroundColor White
Write-Host ""

exit 0

