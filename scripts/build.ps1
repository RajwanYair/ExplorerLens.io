#Requires -Version 7.0

<#
.SYNOPSIS
    DarkThumbs build script with logging and monitoring support

.DESCRIPTION
    Builds DarkThumbs shell extension and Engine library
    - Enforces x64 only
    - Pipes output to logs for VS Code monitoring
    - Supports clean builds
    - Validates tool availability

.PARAMETER Configuration
    Build configuration (Debug or Release)

.PARAMETER Clean
    Perform clean build (removes all build artifacts)

.PARAMETER BuildEngine
    Build Engine library first (default: true)

.PARAMETER SkipVerification
    Skip output file verification

.EXAMPLE
    .\build.ps1
    # Standard Release build

.EXAMPLE
    .\build.ps1 -Configuration Debug -Clean
    # Clean Debug build

.EXAMPLE
    .\build.ps1 -Configuration Release -BuildEngine:$false
    # Build only shell extension (Engine already built)
#>

[CmdletBinding()]
param(
    [Parameter()]
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [Parameter()]
    [switch]$Clean,

    [Parameter()]
    [switch]$BuildEngine = $true,

    [Parameter()]
    [switch]$SkipVerification
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Constants
$WorkspaceRoot = $PSScriptRoot | Split-Path -Parent
$LogDir = Join-Path $WorkspaceRoot "build-logs"
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$LogFile = Join-Path $LogDir "build_${Timestamp}.log"

# Ensure log directory exists
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

# Helper function for logging
function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    Add-Content -Path $LogFile -Value $logMessage
}

# Start build
Write-Log "========================================" 
Write-Log "DarkThumbs Build Script"
Write-Log "Configuration: $Configuration"
Write-Log "Clean Build: $Clean"
Write-Log "Build Engine: $BuildEngine"
Write-Log "Log File: $LogFile"
Write-Log "========================================"

# Verify tools
Write-Log "Verifying build tools..."

$requiredTools = @{
    "msbuild" = "MSBuild (Visual Studio Build Tools)"
    "cmake" = "CMake"
}

if ($BuildEngine) {
    $requiredTools["cmake"] = "CMake"
}

foreach ($tool in $requiredTools.Keys) {
    $found = Get-Command $tool -ErrorAction SilentlyContinue
    if (-not $found) {
        Write-Log "ERROR: $($requiredTools[$tool]) not found in PATH" "ERROR"
        Write-Log "Please install required tools (see .github/docs/WINDOWS_BUILD_TOOLS.md)" "ERROR"
        exit 1
    }
    Write-Log "✓ $tool found at: $($found.Source)"
}

# Change to workspace root
Set-Location $WorkspaceRoot
Write-Log "Working directory: $WorkspaceRoot"

# Clean build artifacts
if ($Clean) {
    Write-Log "Cleaning build artifacts..."
    
    $cleanPaths = @(
        "build",
        "x64",
        "CBXShell\x64",
        "CBXManager\x64",
        "Engine\Release",
        "Engine\Debug"
    )
    
    foreach ($path in $cleanPaths) {
        $fullPath = Join-Path $WorkspaceRoot $path
        if (Test-Path $fullPath) {
            Write-Log "  Removing: $fullPath"
            Remove-Item -Recurse -Force -Path $fullPath -ErrorAction SilentlyContinue
        }
    }
    
    Write-Log "Clean complete"
}

# Build Engine library
if ($BuildEngine) {
    Write-Log "========================================" 
    Write-Log "Building Engine library..."
    Write-Log "========================================"
    
    $engineDir = Join-Path $WorkspaceRoot "Engine"
    $engineBuildDir = Join-Path $engineDir $Configuration
    
    Set-Location $engineDir
    
    try {
        Write-Log "Configuring Engine with CMake..."
        
        # Detect available VS generator
        $vsGenerators = @(
            @{Name = "Visual Studio 18 2026"; Year = "2026"; Version = "18"},
            @{Name = "Visual Studio 17 2022"; Year = "2022"; Version = "17"},
            @{Name = "Visual Studio 16 2019"; Year = "2019"; Version = "16"}
        )
        
        $generator = $null
        foreach ($gen in $vsGenerators) {
            $vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\$($gen.Version)\BuildTools"
            if (Test-Path $vsPath) {
                $generator = $gen.Name
                Write-Log "  Detected: $generator at $vsPath"
                break
            }
        }
        
        if (-not $generator) {
            Write-Log "ERROR: No Visual Studio generator found (2019-2026)" "ERROR"
            exit 1
        }
        
        $cmakeConfigArgs = @(
            "-S", ".",
            "-B", $Configuration,
            "-G", $generator,
            "-A", "x64",
            "-DCMAKE_BUILD_TYPE=$Configuration"
        )
        
        & cmake @cmakeConfigArgs 2>&1 | Tee-Object -FilePath $LogFile -Append
        
        if ($LASTEXITCODE -ne 0) {
            Write-Log "CMake configuration failed (exit code: $LASTEXITCODE)" "ERROR"
            exit 1
        }
        
        Write-Log "Building Engine..."
        & cmake --build $Configuration --config $Configuration -- /m /v:minimal 2>&1 | Tee-Object -FilePath $LogFile -Append
        
        if ($LASTEXITCODE -ne 0) {
            Write-Log "Engine build failed (exit code: $LASTEXITCODE)" "ERROR"
            exit 1
        }
        
        Write-Log "Engine build complete"
        
        # Verify Engine library
        $engineLib = Join-Path $engineBuildDir "$Configuration\DarkThumbsEngine.lib"
        if (-not (Test-Path $engineLib)) {
            Write-Log "ERROR: Engine library not found at $engineLib" "ERROR"
            exit 1
        }
        
        $libSize = (Get-Item $engineLib).Length / 1MB
        Write-Log "✓ DarkThumbsEngine.lib: $([math]::Round($libSize, 2)) MB"
        
    } finally {
        Set-Location $WorkspaceRoot
    }
}

# Build shell extension
Write-Log "========================================" 
Write-Log "Building shell extension..."
Write-Log "========================================"

try {
    $msbuildArgs = @(
        "CBXShell.sln",
        "/p:Configuration=$Configuration",
        "/p:Platform=x64",
        "/m",
        "/v:minimal",
        "/t:Rebuild"
    )
    
    & msbuild @msbuildArgs 2>&1 | Tee-Object -FilePath $LogFile -Append
    
    if ($LASTEXITCODE -ne 0) {
        Write-Log "Shell extension build failed (exit code: $LASTEXITCODE)" "ERROR"
        exit 1
    }
    
    Write-Log "Shell extension build complete"
    
} catch {
    Write-Log "Build error: $_" "ERROR"
    exit 1
}

# Verify output files
if (-not $SkipVerification) {
    Write-Log "========================================" 
    Write-Log "Verifying build outputs..."
    Write-Log "========================================"
    
    $outputDir = Join-Path $WorkspaceRoot "x64\$Configuration"
    $expectedFiles = @(
        "CBXShell.dll",
        "CBXManager.exe"
    )
    
    $allFound = $true
    foreach ($file in $expectedFiles) {
        $fullPath = Join-Path $outputDir $file
        if (Test-Path $fullPath) {
            $fileInfo = Get-Item $fullPath
            $sizeKB = [math]::Round($fileInfo.Length / 1KB, 0)
            Write-Log "✓ $file ($sizeKB KB) - $($fileInfo.LastWriteTime)"
        } else {
            Write-Log "✗ $file NOT FOUND" "ERROR"
            $allFound = $false
        }
    }
    
    if (-not $allFound) {
        Write-Log "Build verification failed - missing output files" "ERROR"
        exit 1
    }
}

# Build summary
Write-Log "========================================" 
Write-Log "BUILD SUCCESSFUL"
Write-Log "Configuration: $Configuration"
Write-Log "Log file: $LogFile"
Write-Log "Output: x64\$Configuration\"
Write-Log "========================================"

Write-Host "`n✅ Build complete! Open log file in VS Code to review details:" -ForegroundColor Green
Write-Host "   $LogFile`n" -ForegroundColor Cyan

exit 0
