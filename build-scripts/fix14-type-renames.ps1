# fix14-type-renames.ps1 — Fix remaining cascade type rename errors in EngineTests.cpp
# Run from the workspace root OR any directory (uses $PSScriptRoot)

$root = Split-Path -Parent $PSScriptRoot
Write-Host "Root: $root"

# ─── EngineTests.cpp ─────────────────────────────────────────────────────────

$f = Join-Path $root "Engine\Tests\EngineTests.cpp"
Write-Host "Patching: $f"

$lines = [System.IO.File]::ReadAllLines($f, [System.Text.Encoding]::UTF8)
$n = $lines.Count
$changed = 0

for ($i = 0; $i -lt $n; $i++) {
    $orig = $lines[$i]
    $line = $orig

    # Fix 1 ─ UpdateChannel:: → EngineUpdateChannel:: (UpdateEngine tests, lines 5739/5741)
    # UpdateChannel was renamed to EngineUpdateChannel in UpdateEngine.h
    $line = $line -replace '\bUpdateChannel::', 'EngineUpdateChannel::'

    # Fix 2 ─ ProxyConfig → DiagnosticsProxyConfig (NetworkDiagnostics.h)
    $line = $line -replace '\bProxyConfig\b', 'DiagnosticsProxyConfig'

    # Fix 3 ─ UpdateCheckResult → AutoUpdateCheckResult (AutoUpdateEngine.h)
    $line = $line -replace '\bUpdateCheckResult\b', 'AutoUpdateCheckResult'

    # Fix 4 ─ AccessibilityFeature → PipelineA11yFeature (AccessibilityPipeline.h)
    # Protect: (?<![:\w]) ensures we don't touch "Core::AccessibilityFeature" or similar
    $line = $line -replace '(?<![:\w])AccessibilityFeature\b', 'PipelineA11yFeature'

    # Fix 5 ─ HighContrastTheme → PipelineHCTheme (AccessibilityPipeline.h)
    # Protect: Core::HighContrastTheme stays unchanged (preceded by ':')
    $line = $line -replace '(?<![:\w])HighContrastTheme\b', 'PipelineHCTheme'

    # Fix 6 ─ unqualified CloudProvider (cast only) → StorageCloudProvider
    # ExplorerLens::Cloud::CloudProvider must NOT be changed (qualified)
    # Only matches bare unqualified CloudProvider (not preceded by ':')
    $line = $line -replace '(?<![:\w])CloudProvider\b(?!:)', 'StorageCloudProvider'

    # Fix 7 ─ CloudFileInfo (unqualified) → StorageCloudFileInfo
    # ExplorerLens::Cloud::CloudFileInfo must NOT be changed
    $line = $line -replace '(?<![:\w])CloudFileInfo\b', 'StorageCloudFileInfo'

    # Fix 8 ─ unqualified HealthStatus:: → EndpointHealthStatus::
    # Protects: COMHealthStatus, ShellHealthStatus, DeployHealthStatus, ExplorerLens::Core::HealthStatus
    $line = $line -replace '(?<![:\w])HealthStatus::', 'EndpointHealthStatus::'

    # Fix 9 ─ ExplorerLens::Plugin::SandboxPolicy:: → ExplorerLens::Plugin::SandboxPolicySpec::
    # Static factory methods: Strict()/Standard()/Developer()
    # NOTE: unqualified SandboxPolicy::Full (PluginMarketplaceUnified) is NOT changed
    $line = $line -replace 'ExplorerLens::Plugin::SandboxPolicy::', 'ExplorerLens::Plugin::SandboxPolicySpec::'

    # Fix 10 ─ ExplorerLens::Plugin::PluginState:: → ExplorerLens::Plugin::ValidationPluginState::
    # (ValidationPluginState is in ExplorerLens::Plugin, PluginActivation.h is in ExplorerLens::Engine::Plugin)
    $line = $line -replace 'ExplorerLens::Plugin::PluginState::', 'ExplorerLens::Plugin::ValidationPluginState::'

    # Fix 11 ─ unqualified PluginState:: → LoaderPluginState:: (PluginLoaderV2.h)
    # Protects ExplorerLens::Engine::Plugin::PluginState:: (preceded by ':')
    $line = $line -replace '(?<![:\w])PluginState::', 'LoaderPluginState::'

    # Fix 12 ─ PluginDescriptor → LoaderPluginDescriptor (PluginLoaderV2.h)
    $line = $line -replace '\bPluginDescriptor\b', 'LoaderPluginDescriptor'

    # Fix 13 ─ TokenType:: → AccessTokenType:: (AccessTokenValidator.h)
    # Only uses are Process/Anonymous which belong to AccessTokenType
    $line = $line -replace '\bTokenType::', 'AccessTokenType::'

    # Fix 14 ─ ExplorerLens::Memory::FormatFamily:: → ExplorerLens::Memory::DirFormatFamily::
    $line = $line -replace 'ExplorerLens::Memory::FormatFamily::', 'ExplorerLens::Memory::DirFormatFamily::'

    # Fix 15 ─ ExplorerLens::ValidatorArtifactType:: → ExplorerLens::Engine::ValidatorArtifactType::
    # ValidatorArtifactType is in ExplorerLens::Engine (CIValidator.h), not ExplorerLens
    $line = $line -replace 'ExplorerLens::ValidatorArtifactType::', 'ExplorerLens::Engine::ValidatorArtifactType::'

    # Fix 16 ─ VerifyStatus:: → ReproBuildVerifyStatus:: (ReproducibleBuildVerifier.h)
    $line = $line -replace '\bVerifyStatus::', 'ReproBuildVerifyStatus::'

    # Fix 17 ─ ExplorerLens::SemanticVersion:: → ExplorerLens::DetectorSemVer::
    # (VersionDriftDetector.h — takes std::string, not wstring)
    $line = $line -replace 'ExplorerLens::SemanticVersion::', 'ExplorerLens::DetectorSemVer::'

    # Fix 18 ─ unqualified DriftSeverity:: → GateDriftSeverity::
    # (ExplorerLens::VersionDrift namespace has GateDriftSeverity with None/Critical)
    # Protects ExplorerLens::DriftSeverity:: (qualified, preceded by ':')
    $line = $line -replace '(?<![:\w])DriftSeverity::', 'GateDriftSeverity::'

    # Fix 19 ─ IPCMessageType:: → HostIPCMessageType:: (PluginHostIPC.h)
    $line = $line -replace '\bIPCMessageType::', 'HostIPCMessageType::'

    if ($line -ne $orig) {
        $changed++
        $lines[$i] = $line
    }
}

[System.IO.File]::WriteAllLines($f, $lines, [System.Text.Encoding]::UTF8)
Write-Host "EngineTests.cpp: $changed lines changed out of $n"
Write-Host "Done."
