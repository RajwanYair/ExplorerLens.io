#Requires -Version 7.0

<#
.SYNOPSIS
    Verify all required build tools are installed and accessible

.DESCRIPTION
    Checks for presence and versions of:
    - Visual Studio Build Tools / MSBuild
    - CMake
    - Ninja
    - Git
    - Python (optional)
    - 7-Zip (optional)
#>

Set-StrictMode -Version Latest

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Build Tools Verification" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$allToolsFound = $true

# Define required and optional tools
$tools = @{
    "Required" = @{
        "cmake"   = @{
            "Name"       = "CMake"
            "Command"    = "cmake --version"
            "MinVersion" = "3.28"
        }
        "msbuild" = @{
            "Name"       = "MSBuild"
            "Command"    = "msbuild /version"
            "MinVersion" = "17.0"
        }
        "git"     = @{
            "Name"       = "Git"
            "Command"    = "git --version"
            "MinVersion" = "2.40"
        }
    }
    "Optional" = @{
        "ninja"  = @{
            "Name"       = "Ninja"
            "Command"    = "ninja --version"
            "MinVersion" = "1.11"
        }
        "python" = @{
            "Name"       = "Python"
            "Command"    = "python --version"
            "MinVersion" = "3.12"
        }
        "7z"     = @{
            "Name"       = "7-Zip"
            "Command"    = '& "C:\Program Files\7-Zip\7z.exe" | Select-String "7-Zip"'
            "MinVersion" = "23.0"
        }
    }
}

function Test-Tool {
    param(
        [string]$ToolName,
        [hashtable]$ToolInfo,
        [bool]$IsRequired
    )
    
    Write-Host "Checking $($ToolInfo.Name)..." -NoNewline
    
    try {
        $found = Get-Command $ToolName -ErrorAction SilentlyContinue
        
        if ($found) {
            Write-Host " ✓ FOUND" -ForegroundColor Green
            Write-Host "  Path: $($found.Source)" -ForegroundColor Gray
            
            # Get version
            try {
                $versionOutput = Invoke-Expression $ToolInfo.Command 2>&1 | Select-Object -First 5
                $versionLine = $versionOutput | Select-String -Pattern "\d+\.\d+" | Select-Object -First 1
                
                if ($versionLine) {
                    Write-Host "  Version: $versionLine" -ForegroundColor Gray
                }
            } catch {
                Write-Host "  (version check failed)" -ForegroundColor Yellow
            }
            
            return $true
        } else {
            if ($IsRequired) {
                Write-Host " ✗ NOT FOUND (REQUIRED)" -ForegroundColor Red
            } else {
                Write-Host " - Not found (optional)" -ForegroundColor Yellow
            }
            return $false
        }
    } catch {
        if ($IsRequired) {
            Write-Host " ✗ ERROR: $_" -ForegroundColor Red
        } else {
            Write-Host " - Error checking (optional)" -ForegroundColor Yellow
        }
        return $false
    }
}

# Check required tools
Write-Host "REQUIRED TOOLS:" -ForegroundColor Cyan
Write-Host "---------------" -ForegroundColor Cyan
foreach ($tool in $tools.Required.Keys) {
    $found = Test-Tool -ToolName $tool -ToolInfo $tools.Required[$tool] -IsRequired $true
    if (-not $found) {
        $allToolsFound = $false
    }
    Write-Host ""
}

# Check optional tools
Write-Host "`nOPTIONAL TOOLS:" -ForegroundColor Cyan
Write-Host "---------------" -ForegroundColor Cyan
foreach ($tool in $tools.Optional.Keys) {
    Test-Tool -ToolName $tool -ToolInfo $tools.Optional[$tool] -IsRequired $false
    Write-Host ""
}

# Check Visual Studio specifically
Write-Host "VISUAL STUDIO BUILD TOOLS:" -ForegroundColor Cyan
Write-Host "-------------------------" -ForegroundColor Cyan

# Check for VS Build Tools (try multiple versions)
$vsPaths = @(
    "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools",   # VS 2026 (v145)
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools", # VS 2022 (v143)
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"  # VS 2019 (v142)
)

$vsFound = $false
foreach ($vsPath in $vsPaths) {
    if (Test-Path $vsPath) {
        Write-Host "✓ Found at: $vsPath" -ForegroundColor Green
        $vsFound = $true
        
        # Check for vcvars64.bat
        $vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
        if (Test-Path $vcvarsPath) {
            Write-Host "✓ vcvars64.bat found" -ForegroundColor Green
        } else {
            Write-Host "⚠ vcvars64.bat not found (may need C++ workload)" -ForegroundColor Yellow
        }
        break
    }
}

if (-not $vsFound) {
    Write-Host "✗ Visual Studio Build Tools not found at any expected location" -ForegroundColor Red
    Write-Host "  Tried: VS 2026 (18\BuildTools), VS 2022, VS 2019" -ForegroundColor Gray
    $allToolsFound = $false
}

# PATH check
Write-Host "`nPATH VERIFICATION:" -ForegroundColor Cyan
Write-Host "------------------" -ForegroundColor Cyan
$pathDirs = $env:Path -split ';' | Where-Object { $_ -ne '' }
Write-Host "PATH contains $($pathDirs.Count) directories" -ForegroundColor Gray

$criticalPaths = @(
    "*CMake*",
    "*MSBuild*",
    "*Git*"
)

Write-Host "`nCritical paths in PATH:" -ForegroundColor Cyan
foreach ($pattern in $criticalPaths) {
    $matches = $pathDirs | Where-Object { $_ -like $pattern }
    if ($matches) {
        foreach ($match in $matches) {
            Write-Host "  ✓ $match" -ForegroundColor Green
        }
    } else {
        Write-Host "  ⚠ No match for $pattern" -ForegroundColor Yellow
    }
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
if ($allToolsFound) {
    Write-Host "✓ ALL REQUIRED TOOLS FOUND" -ForegroundColor Green
    Write-Host "========================================`n" -ForegroundColor Cyan
    Write-Host "You can proceed with building:" -ForegroundColor White
    Write-Host "  .\scripts\build.ps1`n" -ForegroundColor Gray
    exit 0
} else {
    Write-Host "✗ MISSING REQUIRED TOOLS" -ForegroundColor Red
    Write-Host "========================================`n" -ForegroundColor Cyan
    Write-Host "Install missing tools:" -ForegroundColor White
    Write-Host "  See .github\docs\WINDOWS_BUILD_TOOLS.md" -ForegroundColor Gray
    Write-Host "  Or run: winget install <TOOL_NAME>`n" -ForegroundColor Gray
    exit 1
}
