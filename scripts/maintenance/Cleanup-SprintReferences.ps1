<#
.SYNOPSIS
    Comprehensive cleanup of sprint/phase/task references from the ExplorerLens project.
    Renames sprint-named files to feature-based names, cleans sprint comments.
#>

param(
    [switch]$DryRun,
    [switch]$RenameOnly,
    [switch]$CommentsOnly
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

Write-Host "ExplorerLens Sprint Cleanup Script" -ForegroundColor Cyan
Write-Host "Project root: $projectRoot" -ForegroundColor DarkGray
Write-Host ""

#==============================================================================
# Phase 1: Rename sprint-named test files in Engine/Tests/
#==============================================================================

$engineTestRenames = @{
    "Sprint150_PluginTestMatrix.cpp"      = "PluginTestMatrixTests.cpp"
    "Sprint151_PluginSandboxPolicy.cpp"   = "PluginSandboxPolicyTests.cpp"
    "Sprint152_PluginCompatKitV2.cpp"     = "PluginCompatKitTests.cpp"
    "Sprint153_PluginReferencePack.cpp"   = "PluginReferencePackTests.cpp"
    "Sprint154_PluginTrustChain.cpp"      = "PluginTrustChainTests.cpp"
    "Sprint155_ARM64BuildConfig.cpp"      = "ARM64BuildConfigTests.cpp"
    "Sprint156_ARM64LibraryMatrix.cpp"    = "ARM64LibraryMatrixTests.cpp"
    "Sprint157_ARM64RuntimeValidator.cpp" = "ARM64RuntimeValidatorTests.cpp"
    "Sprint158_ARM64PerfBaseline.cpp"     = "ARM64PerfBaselineTests.cpp"
    "Sprint159_ARM64CIIntegration.cpp"    = "ARM64CIIntegrationTests.cpp"
    "Sprint160_JPEG2000Decoder.cpp"       = "JPEG2000DecoderTests.cpp"
    "Sprint161_CADFormat.cpp"             = "CADFormatDecoderTests.cpp"
    "Sprint162_GLTFModel.cpp"             = "GLTFModelDecoderTests.cpp"
    "Sprint163_Scientific.cpp"            = "ScientificDecoderTests.cpp"
    "Sprint164_FallbackEngine.cpp"        = "FallbackEngineTests.cpp"
    "Sprint165_ArchiveMemory.cpp"         = "ArchiveMemoryTests.cpp"
    "Sprint166_ZeroCopy.cpp"              = "ZeroCopyPipelineTests.cpp"
    "Sprint167_AdaptiveCache.cpp"         = "AdaptiveCacheBudgetTests.cpp"
    "Sprint168_HotMode.cpp"               = "HotModeDirectoryTests.cpp"
    "Sprint169_MemPressure.cpp"           = "MemoryPressureTests.cpp"
    "Sprint170_MatrixValidation.cpp"      = "MatrixValidationTests.cpp"
    "Sprint171_InstallerLifecycle.cpp"    = "InstallerLifecycleTests.cpp"
    "Sprint172_ReleaseGate.cpp"           = "ReleaseGateTests.cpp"
    "Sprint173_DocSync.cpp"               = "DocumentationSyncTests.cpp"
    "Sprint174_ProgramClosure.cpp"        = "ProgramClosureTests.cpp"
}

# Sprint-named test files in tests/ directory
$testsRenames = @{
    "Sprint6_IsolationTests.cpp"              = "IsolationStabilityTests.cpp"
    "Sprint7_Windows11Compatibility.cpp"      = "Windows11CompatibilityTests.cpp"
    "Sprint8_GUIHardening.cpp"                = "GUIHardeningTests.cpp"
    "Sprint9_VersionNormalization.cpp"        = "VersionNormalizationTests.cpp"
    "Sprint10_ReleaseGovernance.cpp"          = "ReleaseGovernanceTests.cpp"
    "Sprint11_PluginSystemActivation.cpp"     = "PluginSystemActivationTests.cpp"
    "Sprint12_Observability.cpp"              = "ObservabilityTests.cpp"
    "Sprint23_AIEnhancementTests.cpp"         = "AIEnhancementTests.cpp"
    "Sprint24_MSIXPackaging.cpp"              = "MSIXPackagingTests.cpp"
    "Sprint25_OpenImageIO.cpp"                = "OpenImageIOTests.cpp"
    "Sprint26_CloudIntegration.cpp"           = "CloudIntegrationTests.cpp"
    "Sprint27_AdvancedCaching.cpp"            = "AdvancedCachingTests.cpp"
    "Sprint28_VideoEnhancement.cpp"           = "VideoEnhancementTests.cpp"
    "Sprint29_PluginMarketplace.cpp"          = "PluginMarketplaceTests.cpp"
    "Sprint30_Accessibility.cpp"              = "AccessibilityI18nTests.cpp"
    "Sprint31_EnterpriseDeployment.cpp"       = "EnterpriseDeploymentTests.cpp"
    "Sprint32_PerformancePolish.cpp"          = "PerformancePolishTests.cpp"
    "Sprint33_CrashIntelligence.cpp"          = "CrashIntelligenceTests.cpp"
    "Sprint34_SupplyChainSecurity.cpp"        = "SupplyChainSecurityTests.cpp"
    "Sprint35_USNCacheInvalidation.cpp"       = "USNCacheInvalidationTests.cpp"
    "Sprint36_ModularCodecs.cpp"              = "ModularCodecsTests.cpp"
    "Sprint37_ContextMenuShellUX.cpp"         = "ContextMenuShellUXTests.cpp"
    "Sprint38_AnimatedThumbnails.cpp"         = "AnimatedThumbnailsTests.cpp"
    "Sprint39_ArchiveGridPreview.cpp"         = "ArchiveGridPreviewTests.cpp"
    "Sprint40_ColorSpaceHDR.cpp"              = "ColorSpaceHDRTests.cpp"
    "Sprint41_DuplicateDetection.cpp"         = "DuplicateDetectionTests.cpp"
    "Sprint42_PortableMode.cpp"               = "PortableModeTests.cpp"
    "Sprint43_BatchProcessing.cpp"            = "BatchProcessingTests.cpp"
    "Sprint44_NetworkThumbnails.cpp"          = "NetworkThumbnailsTests.cpp"
    "Sprint45_PreviewPane.cpp"                = "PreviewPaneTests.cpp"
    "Sprint46_FormatConversion.cpp"           = "FormatConversionTests.cpp"
    "Sprint47_AccessibilityI18n.cpp"          = "AccessibilityFrameworkTests.cpp"
    "Sprint48_TelemetryDashboard.cpp"         = "TelemetryDashboardTests.cpp"
    "Sprint49_ReleasePackaging.cpp"           = "ReleasePackagingTests.cpp"
    "Sprint53_BuildValidation.cpp"            = "BuildValidationTests.cpp"
    "Sprint125_VersionDriftGate.cpp"          = "VersionDriftGateTests.cpp"
    "Sprint126_ETWSinkComplete.cpp"           = "ETWSinkCompleteTests.cpp"
    "Sprint127_COMApartmentAudit.cpp"         = "COMApartmentAuditTests.cpp"
    "Sprint128_PluginRuntimeValidation.cpp"   = "PluginRuntimeValidationTests.cpp"
    "Sprint129_DirectoryFormatProfiler.cpp"   = "DirectoryFormatProfilerTests.cpp"
    "Sprint130_DecoderHotsetManager.cpp"      = "DecoderHotsetManagerTests.cpp"
    "Sprint131_BufferPoolAllocator.cpp"       = "BufferPoolAllocatorTests.cpp"
    "Sprint132_ExplorerWorkScheduler.cpp"     = "ExplorerWorkSchedulerTests.cpp"
    "Sprint133_JPEG2000Decoder.cpp"           = "JPEG2000DecoderGTests.cpp"
    "Sprint134_KTXTextureDecoder.cpp"         = "KTXTextureDecoderTests.cpp"
    "Sprint135_JXRWICDecoder.cpp"             = "JXRWICDecoderTests.cpp"
    "Sprint136_EBookCoverExtractor.cpp"       = "EBookCoverExtractorTests.cpp"
    "Sprint137_ContinuousFuzzEngine.cpp"      = "ContinuousFuzzEngineTests.cpp"
    "Sprint138_CompatibilityMatrix.cpp"       = "CompatibilityMatrixTests.cpp"
    "Sprint139_MSILifecycleRunner.cpp"        = "MSILifecycleRunnerTests.cpp"
    "Sprint140_MemorySoakValidator.cpp"       = "MemorySoakValidatorTests.cpp"
    "Sprint141_DarkModeManagerV2.cpp"         = "DarkModeManagerTests.cpp"
    "Sprint142_PerMonitorDPIManager.cpp"      = "PerMonitorDPIManagerTests.cpp"
    "Sprint143_DecoderHealthDashboard.cpp"    = "DecoderHealthDashboardTests.cpp"
    "Sprint144_DiagnosticsExporter.cpp"       = "DiagnosticsExporterTests.cpp"
    "Sprint145_VersionDriftDetector.cpp"      = "VersionDriftDetectorTests.cpp"
    "Sprint146_PerfRegressionGate.cpp"        = "PerfRegressionGateTests.cpp"
    "Sprint147_ReproducibleBuildVerifier.cpp" = "ReproducibleBuildVerifierTests.cpp"
    "Sprint148_ReleaseReadinessDashboard.cpp" = "ReleaseReadinessDashboardTests.cpp"
}

Write-Host "=== Phase 1: Renaming sprint-named test files ===" -ForegroundColor Yellow

# Rename Engine/Tests/ files
$engineTestDir = Join-Path $projectRoot "Engine\Tests"
$renamedCount = 0
foreach ($entry in $engineTestRenames.GetEnumerator()) {
    $oldPath = Join-Path $engineTestDir $entry.Key
    $newPath = Join-Path $engineTestDir $entry.Value
    if (Test-Path $oldPath) {
        if ($DryRun) {
            Write-Host "  [DRY] $($entry.Key) -> $($entry.Value)" -ForegroundColor DarkGray
        } else {
            Rename-Item -Path $oldPath -NewName $entry.Value -Force
            Write-Host "  ✓ $($entry.Key) -> $($entry.Value)" -ForegroundColor Green
        }
        $renamedCount++
    }
}

# Rename tests/ files
$testsDir = Join-Path $projectRoot "tests"
foreach ($entry in $testsRenames.GetEnumerator()) {
    $oldPath = Join-Path $testsDir $entry.Key
    $newPath = Join-Path $testsDir $entry.Value
    if (Test-Path $oldPath) {
        if ($DryRun) {
            Write-Host "  [DRY] $($entry.Key) -> $($entry.Value)" -ForegroundColor DarkGray
        } else {
            Rename-Item -Path $oldPath -NewName $entry.Value -Force
            Write-Host "  ✓ $($entry.Key) -> $($entry.Value)" -ForegroundColor Green
        }
        $renamedCount++
    }
}

# CBX→LENS rename migration completed (v15.0.0)

Write-Host "  Renamed $renamedCount files" -ForegroundColor Cyan

if ($RenameOnly) { exit 0 }

#==============================================================================
# Phase 2: Update CMakeLists.txt references
#==============================================================================

Write-Host ""
Write-Host "=== Phase 2: Updating CMakeLists.txt references ===" -ForegroundColor Yellow

$engineTestsCmake = Join-Path $engineTestDir "CMakeLists.txt"
if (Test-Path $engineTestsCmake) {
    $content = Get-Content $engineTestsCmake -Raw

    # Replace sprint-named references with new names
    foreach ($entry in $engineTestRenames.GetEnumerator()) {
        $content = $content.Replace($entry.Key, $entry.Value)
    }

    # Remove "# Sprint" comments
    $content = $content -replace '(?m)^\s*#\s*Sprint \d+.*$\n?', ''
    $content = $content -replace '# Sprint \d+:.*', ''

    if (-not $DryRun) {
        Set-Content -Path $engineTestsCmake -Value $content -NoNewline
        Write-Host "  ✓ Updated Engine/Tests/CMakeLists.txt" -ForegroundColor Green
    }
}

#==============================================================================
# Phase 3: Clean sprint comments from source files
#==============================================================================

if ($CommentsOnly -or -not $RenameOnly) {
    Write-Host ""
    Write-Host "=== Phase 3: Cleaning sprint comments from source files ===" -ForegroundColor Yellow

    $extensions = @("*.cpp", "*.h", "*.ps1", "*.cmake")
    $excludeDirs = @("build", "build-vcpkg", "external", "x64", "packages", ".git", "gtest", "build-logs")

    $cleanedCount = 0

    foreach ($ext in $extensions) {
        $files = Get-ChildItem -Path $projectRoot -Filter $ext -Recurse -File | Where-Object {
            $path = $_.FullName
            -not ($excludeDirs | Where-Object { $path -like "*\$_\*" })
        }

        foreach ($file in $files) {
            $content = Get-Content $file.FullName -Raw
            $original = $content

            # Pattern 1: "// Sprint NNN: ..." at the start of a line (standalone comment)
            # Only remove if the sprint comment is the ENTIRE line (not inline)
            $content = $content -replace '(?m)^(\s*)//\s*Sprint \d+[\.\d]*\s*[:—\-–]\s*[^\r\n]*\r?\n', ''

            # Pattern 2: "// Part of Sprint NNN: ..."
            $content = $content -replace '(?m)^(\s*)//\s*Part of Sprint \d+[\.\d]*\s*[:—\-–]\s*[^\r\n]*\r?\n', ''

            # Pattern 3: "# Sprint NNN: ..." in scripts
            $content = $content -replace '(?m)^(\s*)#\s*Sprint \d+[\.\d]*\s*[:—\-–]\s*[^\r\n]*\r?\n', ''

            # Pattern 4: Sprint references in inline comments (remove the sprint part only)
            $content = $content -replace '\(Sprint \d+[\.\d]*\)', ''
            $content = $content -replace '\(Sprint \d+[\.\d]* .*?\)', ''

            # Pattern 5: "Sprint NNN" in description strings
            $content = $content -replace '(?<=\bfor\b\s+)Sprint \d+[\.\d]*', 'this module'

            if ($content -ne $original) {
                if (-not $DryRun) {
                    Set-Content -Path $file.FullName -Value $content -NoNewline
                }
                $cleanedCount++
            }
        }
    }

    Write-Host "  Cleaned sprint comments from $cleanedCount files" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "Sprint cleanup complete!" -ForegroundColor Green
