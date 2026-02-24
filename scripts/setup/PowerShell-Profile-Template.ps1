# PowerShell Profile Configuration for ExplorerLens Development
# This file should be placed at: $PROFILE.CurrentUserAllHosts
# Location: C:\Users\<username>\OneDrive - Intel Corporation\Documents\PowerShell\profile.ps1

# Visual Studio BuildTools 2026 Configuration
$Global:VSBuildToolsConfig = @{
    # Visual Studio paths (in order of preference)
    # Using environment variables for cross-system compatibility
    VSPaths = @(
        "$env:ProgramFiles(x86)\Microsoft Visual Studio\18\BuildTools",
        "$env:ProgramFiles\Microsoft Visual Studio\2026\BuildTools",
        "$env:ProgramFiles(x86)\Microsoft Visual Studio\2022\BuildTools",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community"
    )
    
    # Tool paths (relative to VS root)
    MSBuildPath = "MSBuild\Current\Bin\MSBuild.exe"
    MSBuildAltPath = "MSBuild\Current\Bin\amd64\MSBuild.exe"
    VCVarsPath = "VC\Auxiliary\Build\vcvarsall.bat"
    
    # Additional tools (using environment variables)
    CMakePath = "$env:USERPROFILE\scoop\shims\cmake.exe"
    NinjaPath = "$env:USERPROFILE\scoop\shims\ninja.exe"
    GitPath = "$env:USERPROFILE\scoop\shims\git.exe"
    PythonPath = "$env:LOCALAPPDATA\Microsoft\WindowsApps\python.exe"
}

# Initialize Visual Studio Build Environment
function Initialize-VSBuildTools {
    param(
        [string]$Architecture = "x64",
        [switch]$Force,
        [switch]$Quiet
    )
    
    # Check if already initialized
    if ((Get-Command cl.exe -ErrorAction SilentlyContinue) -and -not $Force) {
        if (-not $Quiet) {
            Write-Host "[OK] VS Build Tools already initialized" -ForegroundColor Green
        }
        return $true
    }
    
    # Find Visual Studio installation
    $vsPath = $null
    foreach ($path in $Global:VSBuildToolsConfig.VSPaths) {
        if (Test-Path $path) {
            $vsPath = $path
            if (-not $Quiet) {
                Write-Host "[OK] Found Visual Studio at: $vsPath" -ForegroundColor Green
            }
            break
        }
    }
    
    if (-not $vsPath) {
        Write-Warning "[FAIL] Visual Studio not found in any expected location"
        return $false
    }
    
    # Set MSBuild path
    $msbuildPath = Join-Path $vsPath $Global:VSBuildToolsConfig.MSBuildPath
    if (-not (Test-Path $msbuildPath)) {
        $msbuildPath = Join-Path $vsPath $Global:VSBuildToolsConfig.MSBuildAltPath
    }
    
    if (Test-Path $msbuildPath) {
        $msbuildDir = Split-Path $msbuildPath
        if ($env:PATH -notlike "*$msbuildDir*") {
            $env:PATH = "$msbuildDir;$env:PATH"
        }
        if (-not $Quiet) {
            Write-Host "[OK] MSBuild added to PATH" -ForegroundColor Green
        }
    }
    
    # Initialize MSVC environment
    $vcvarsPath = Join-Path $vsPath $Global:VSBuildToolsConfig.VCVarsPath
    if (Test-Path $vcvarsPath) {
        if (-not $Quiet) {
            Write-Host "[...] Initializing MSVC environment..." -ForegroundColor Yellow
        }
        
        # Run vcvarsall and capture environment
        $tempFile = [System.IO.Path]::GetTempFileName()
        cmd /c "`"$vcvarsPath`" $Architecture && set > `"$tempFile`""
        
        Get-Content $tempFile | ForEach-Object {
            if ($_ -match "^([^=]+)=(.*)$") {
                $varName = $matches[1]
                $varValue = $matches[2]
                Set-Item -Path "env:$varName" -Value $varValue -Force
            }
        }
        
        Remove-Item $tempFile -Force -ErrorAction SilentlyContinue
        
        if (-not $Quiet) {
            Write-Host "[OK] MSVC environment initialized ($Architecture)" -ForegroundColor Green
        }
        return $true
    } else {
        Write-Warning "[WARN]  vcvarsall.bat not found at: $vcvarsPath"
        return $false
    }
}

# Add development tools to PATH
function Add-DevelopmentTools {
    param([switch]$Quiet)
    
    $toolsAdded = 0
    
    # Add Scoop shims directory
    $scoopShims = "$env:USERPROFILE\scoop\shims"
    if ((Test-Path $scoopShims) -and ($env:PATH -notlike "*$scoopShims*")) {
        $env:PATH = "$scoopShims;$env:PATH"
        $toolsAdded++
        if (-not $Quiet) {
            Write-Host "[OK] Scoop tools added to PATH" -ForegroundColor Green
        }
    }
    
    # Verify critical tools
    $tools = @("git", "cmake", "ninja", "python")
    $missingTools = @()
    
    foreach ($tool in $tools) {
        if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
            $missingTools += $tool
        }
    }
    
    if ($missingTools.Count -gt 0 -and -not $Quiet) {
        Write-Warning "[WARN]  Missing tools: $($missingTools -join ', ')"
        Write-Host "   Run: scoop install $($missingTools -join ' ')" -ForegroundColor Yellow
    }
    
    return $toolsAdded
}

# ExplorerLens-specific functions
function Set-ExplorerLensEnvironment {
    param(
        [string]$ProjectPath = "$env:USERPROFILE\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens",
        [switch]$InitializeVS,
        [switch]$Quiet
    )
    
    if (-not $Quiet) {
        Write-Host ""
        Write-Host "+===============================================================+" -ForegroundColor Cyan
        Write-Host "|         ExplorerLens Development Environment                    |" -ForegroundColor Cyan
        Write-Host "+===============================================================+" -ForegroundColor Cyan
        Write-Host ""
    }
    
    # Set project location
    $Global:ExplorerLensPath = $ProjectPath
    Set-Location $ProjectPath -ErrorAction SilentlyContinue
    
    # Add development tools
    Add-DevelopmentTools -Quiet:$Quiet
    
    # Initialize VS Build Tools if requested
    if ($InitializeVS) {
        Initialize-VSBuildTools -Quiet:$Quiet
    }
    
    if (-not $Quiet) {
        Write-Host "[OK] ExplorerLens environment ready!" -ForegroundColor Green
        Write-Host ""
    }
}

# Quick navigation aliases
function dt { Set-Location $Global:ExplorerLensPath }
function dt-build { Set-Location (Join-Path $Global:ExplorerLensPath "scripts\build") }
function dt-scripts { Set-Location (Join-Path $Global:ExplorerLensPath "scripts") }
function dt-docs { Set-Location (Join-Path $Global:ExplorerLensPath "documentation") }

# Build helpers
function Build-ExplorerLens {
    param(
        [ValidateSet('build', 'clean', 'test', 'install', 'uninstall', 'verify')]
        [string]$Action = 'build',
        [switch]$Initialize
    )
    
    Push-Location $Global:ExplorerLensPath
    
    if ($Initialize) {
        Initialize-VSBuildTools
    }
    
    & ".\explorerlens.ps1" $Action
    
    Pop-Location
}

# Tool verification
function Test-ExplorerLensTools {
    Write-Host ""
    Write-Host "+===============================================================+" -ForegroundColor Cyan
    Write-Host "|         Development Tools Check                               |" -ForegroundColor Cyan
    Write-Host "+===============================================================+" -ForegroundColor Cyan
    Write-Host ""
    
    $tools = @{
        "git" = "Git"
        "cmake" = "CMake"
        "ninja" = "Ninja"
        "python" = "Python"
        "msbuild" = "MSBuild"
        "cl" = "MSVC Compiler"
        "link" = "MSVC Linker"
    }
    
    foreach ($tool in $tools.Keys) {
        $cmd = Get-Command $tool -ErrorAction SilentlyContinue
        if ($cmd) {
            Write-Host "  [OK] $($tools[$tool]): $($cmd.Source)" -ForegroundColor Green
        } else {
            Write-Host "  [FAIL] $($tools[$tool]): NOT FOUND" -ForegroundColor Red
        }
    }
    
    Write-Host ""
}

# Auto-initialize on profile load (silent mode)
Add-DevelopmentTools -Quiet

# Set up prompt with VS indicator
function prompt {
    $hasVS = (Get-Command cl.exe -ErrorAction SilentlyContinue) -ne $null
    $vsIndicator = if ($hasVS) { "[VS]" } else { "" }
    
    $location = Get-Location
    $locationShort = $location.Path.Replace($HOME, "~")
    
    Write-Host "$vsIndicator" -NoNewline -ForegroundColor Green
    Write-Host "PS " -NoNewline -ForegroundColor Cyan
    Write-Host "$locationShort" -NoNewline -ForegroundColor Yellow
    return "> "
}

# Display initialization message
Write-Host ""
Write-Host "ExplorerLens PowerShell Profile Loaded" -ForegroundColor Cyan
Write-Host "Run " -NoNewline
Write-Host "Set-ExplorerLensEnvironment" -NoNewline -ForegroundColor Yellow
Write-Host " to initialize development environment"
Write-Host "Run " -NoNewline
Write-Host "Test-ExplorerLensTools" -NoNewline -ForegroundColor Yellow
Write-Host " to verify all tools"
Write-Host ""

