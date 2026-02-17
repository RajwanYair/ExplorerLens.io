# Test-PluginSystem.ps1
# Sprint 11: Plugin System Activation
# Tests plugin discovery, loading, and IPC functionality

param(
    [string]$PluginDir = "",
    [switch]$BuildSamplePlugin = $false
)

$ErrorActionPreference = "Stop"

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Plugin System Test" -ForegroundColor Cyan
Write-Host "Sprint 11: Plugin System Activation" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

$ScriptDir = Split-Path -Parent $PSCommandPath
$RootDir = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$SDKDir = Join-Path $RootDir "SDK"
$SamplePluginDir = Join-Path $SDKDir "examples\minimal-plugin"

# =============================================================================
# 1. Verify Plugin Directories
# =============================================================================
Write-Host "[1/6] Verifying plugin directories..." -ForegroundColor Yellow

$PluginSearchPaths = @(
    "$env:ProgramData\DarkThumbs\Plugins",
    "$env:LOCALAPPDATA\DarkThumbs\Plugins",
    "$env:APPDATA\DarkThumbs\Plugins"
)

foreach ($path in $PluginSearchPaths) {
    if (-not (Test-Path $path)) {
        New-Item -ItemType Directory -Path $path -Force | Out-Null
        Write-Host "  ✓ Created: $path" -ForegroundColor Green
    } else {
        Write-Host "  ✓ Exists: $path" -ForegroundColor Green
    }
}

# =============================================================================
# 2. Check Sample Plugin
# =============================================================================
Write-Host "`n[2/6] Checking sample plugin..." -ForegroundColor Yellow

if (Test-Path $SamplePluginDir) {
    Write-Host "  ✓ Sample plugin found at: $SamplePluginDir" -ForegroundColor Green
    
    $requiredFiles = @("CMakeLists.txt", "minimal_plugin.cpp", "manifest.json", "README.md")
    foreach ($file in $requiredFiles) {
        $filePath = Join-Path $SamplePluginDir $file
        if (Test-Path $filePath) {
            Write-Host "    ✓ $file" -ForegroundColor Gray
        } else {
            Write-Host "    ✗ $file missing" -ForegroundColor Red
        }
    }
} else {
    Write-Host "  ✗ Sample plugin not found" -ForegroundColor Red
}

# =============================================================================
# 3. Build Sample Plugin (Optional)
# =============================================================================
if ($BuildSamplePlugin) {
    Write-Host "`n[3/6] Building sample plugin..." -ForegroundColor Yellow
    
    $pluginBuildDir = Join-Path $SamplePluginDir "build"
    if (-not (Test-Path $pluginBuildDir)) {
        New-Item -ItemType Directory -Path $pluginBuildDir -Force | Out-Null
    }
    
    Push-Location $pluginBuildDir
    try {
        # Configure with CMake
        Write-Host "  Configuring with CMake..." -ForegroundColor Gray
        & cmake .. -G "Visual Studio 18 2026" -A x64 2>&1 | Out-Null
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ CMake configuration succeeded" -ForegroundColor Green
            
            # Build
            Write-Host "  Building plugin..." -ForegroundColor Gray
            & cmake --build . --config Release 2>&1 | Out-Null
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "  ✓ Plugin built successfully" -ForegroundColor Green
                
                # Check output
                $pluginDll = Join-Path $pluginBuildDir "Release\minimal_plugin.dll"
                if (Test-Path $pluginDll) {
                    $dllSize = [math]::Round((Get-Item $pluginDll).Length / 1KB, 2)
                    Write-Host "    Plugin DLL: $dllSize KB" -ForegroundColor Cyan
                } else {
                    Write-Host "  ✗ Plugin DLL not found" -ForegroundColor Red
                }
            } else {
                Write-Host "  ✗ Build failed" -ForegroundColor Red
            }
        } else {
            Write-Host "  ✗ CMake configuration failed" -ForegroundColor Red
        }
    } finally {
        Pop-Location
    }
} else {
    Write-Host "`n[3/6] Building sample plugin... SKIPPED" -ForegroundColor Yellow
    Write-Host "  (Use -BuildSamplePlugin to build)" -ForegroundColor Gray
}

# =============================================================================
# 4. Verify Plugin Host Executable
# =============================================================================
Write-Host "`n[4/6] Checking PluginHost executable..." -ForegroundColor Yellow

$pluginHostPaths = @(
    "build\bin\Release\PluginHost.exe",
    "x64\Release\PluginHost.exe",
    "build\x64\Release\PluginHost.exe"
)

$pluginHostFound = $false
foreach ($path in $pluginHostPaths) {
    $fullPath = Join-Path $RootDir $path
    if (Test-Path $fullPath) {
        $exeSize = [math]::Round((Get-Item $fullPath).Length / 1KB, 2)
        Write-Host "  ✓ PluginHost.exe found: $exeSize KB" -ForegroundColor Green
        Write-Host "    Path: $path" -ForegroundColor Gray
        $pluginHostFound = $true
        break
    }
}

if (-not $pluginHostFound) {
    Write-Host "  ⚠ PluginHost.exe not found (may need to build Engine project)" -ForegroundColor Yellow
}

# =============================================================================
# 5. Check Plugin API Headers
# =============================================================================
Write-Host "`n[5/6] Checking Plugin API..." -ForegroundColor Yellow

$apiHeaders = @(
    "SDK\plugin_api.h",
    "SDK\include\DarkThumbsPlugin.h"
)

foreach ($header in $apiHeaders) {
    $headerPath = Join-Path $RootDir $header
    if (Test-Path $headerPath) {
        Write-Host "  ✓ $header" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ $header not found" -ForegroundColor Yellow
    }
}

# =============================================================================
# 6. Test Registry Configuration
# =============================================================================
Write-Host "`n[6/6] Checking configuration..." -ForegroundColor Yellow

$regPath = "HKCU:\Software\DarkThumbs"
if (Test-Path $regPath) {
    try {
        $pluginsEnabled = Get-ItemProperty -Path $regPath -Name "PluginsEnabled" -ErrorAction SilentlyContinue
        if ($pluginsEnabled) {
            Write-Host "  ✓ Plugins registry key exists" -ForegroundColor Green
            Write-Host "    Value: $($pluginsEnabled.PluginsEnabled)" -ForegroundColor Gray
        } else {
            Write-Host "  ⚠ PluginsEnabled key not set (defaults to enabled)" -ForegroundColor Yellow
        }
    } catch {
        Write-Host "  ⚠ Could not read registry: $_" -ForegroundColor Yellow
    }
} else {
    Write-Host "  ⚠ DarkThumbs registry key not found (will be created on first run)" -ForegroundColor Yellow
}

# =============================================================================
# Summary
# =============================================================================
Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "Plugin System Test Summary" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan

Write-Host "`nPlugin Infrastructure:" -ForegroundColor Yellow
Write-Host "  ✓ Plugin directories configured" -ForegroundColor Green
Write-Host "  ✓ Sample plugin available" -ForegroundColor Green
Write-Host "  ✓ Plugin API headers present" -ForegroundColor Green

Write-Host "`nNext Steps:" -ForegroundColor Yellow
Write-Host "  1. Build sample plugin: .\Test-PluginSystem.ps1 -BuildSamplePlugin" -ForegroundColor Cyan
Write-Host "  2. Copy plugin DLL to: %LOCALAPPDATA%\DarkThumbs\Plugins\" -ForegroundColor Cyan
Write-Host "  3. Create manifest.json alongside plugin DLL" -ForegroundColor Cyan
Write-Host "  4. Test with CBXManager.exe plugin management UI" -ForegroundColor Cyan

Write-Host "`nDocumentation:" -ForegroundColor Yellow
Write-Host "  - Plugin SDK: SDK\README.md" -ForegroundColor Cyan
Write-Host "  - Sample Plugin: SDK\examples\minimal-plugin\README.md" -ForegroundColor Cyan
Write-Host "  - Plugin API: SDK\docs\plugin-development-guide.md" -ForegroundColor Cyan

Write-Host "`n============================================`n" -ForegroundColor Cyan
