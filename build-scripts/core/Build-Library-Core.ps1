# Build-Library-Core.ps1
# ExplorerLens v7.0 - Unified Build Library Core Functions
# Consolidates common build patterns across all external library scripts
#
# USAGE:
#   . "$PSScriptRoot/../core/Build-Library-Core.ps1"
#   Invoke-CMakeBuild -LibraryName "zlib" -SourceDir $sourceDir -BuildDir $buildDir
#
# Date: February 16, 2026
# Author: ExplorerLens Development Team

$ErrorActionPreference = 'Stop'

# ============================================================================
# Common Build Configuration
# ============================================================================

$Script:BuildConfig = @{
    # Default configurations
    Configurations      = @('Release', 'Debug')
    Platform            = 'x64'
    
    # CMake defaults
    CMakeGenerator      = 'Visual Studio 18 2026'
    CMakeToolset        = 'v145'
    
    # MSBuild defaults
    MSBuildPlatform     = 'x64'
    MSBuildToolsVersion = '18.0'
    
    # Common flags
    RuntimeLibrary      = '/MD'  # MultiThreadedDLL for Release
    RuntimeLibraryDebug = '/MDd'  # MultiThreadedDebugDLL for Debug
    
    # Optimization flags — NOTE: /GL (LTCG) is intentionally excluded.
    # /GL embeds compiler-version-specific IR into .lib files, making them
    # incompatible across MSVC toolset versions (e.g. v145-built .lib fails
    # to link with v143). LTCG is handled at the main Engine link stage instead.
    OptimizationFlags   = @('/O2', '/Oi', '/Ot', '/Ob2')
    
    # Security flags
    SecurityFlags       = @('/GS', '/guard:cf', '/Qspectre')
}

# ============================================================================
# Logging Functions
# ============================================================================

function Write-BuildLog {
    <#
    .SYNOPSIS
        Writes a formatted build log message
    .PARAMETER Message
        The message to log
    .PARAMETER Level
        Log level: Info, Warning, Error, Success
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$Message,
        
        [Parameter()]
        [ValidateSet('Info', 'Warning', 'Error', 'Success')]
        [string]$Level = 'Info'
    )
    
    $timestamp = Get-Date -Format 'HH:mm:ss'
    $prefix = switch ($Level) {
        'Info' { "[$timestamp] INFO:" }
        'Warning' { "[$timestamp] WARN:" }
        'Error' { "[$timestamp] ERROR:" }
        'Success' { "[$timestamp] ✓" }
    }
    
    $color = switch ($Level) {
        'Info' { 'Cyan' }
        'Warning' { 'Yellow' }
        'Error' { 'Red' }
        'Success' { 'Green' }
    }
    
    Write-Host "$prefix $Message" -ForegroundColor $color
}

function Write-BuildHeader {
    <#
    .SYNOPSIS
        Writes a formatted header for build operations
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$Title
    )
    
    Write-Host ""
    Write-Host ("=" * 80) -ForegroundColor Magenta
    Write-Host "  $Title" -ForegroundColor Magenta
    Write-Host ("=" * 80) -ForegroundColor Magenta
    Write-Host ""
}

# ============================================================================
# Tool Discovery Functions
# ============================================================================

function Find-MSBuildPath {
    <#
    .SYNOPSIS
        Finds MSBuild.exe path (VS 2022/2026)
    .OUTPUTS
        String path to MSBuild.exe or $null if not found
    #>
    [CmdletBinding()]
    param()
    
    # Try vswhere first (most reliable)
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
            -property installationPath -version '[17.0,19.0)'
        
        if ($vsPath) {
            $vsCandidates = @(
                (Join-Path $vsPath 'MSBuild\Current\Bin\amd64\MSBuild.exe'),
                (Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe')
            )
            foreach ($msbuild in $vsCandidates) {
                if (Test-Path $msbuild) {
                    Write-BuildLog "Found MSBuild: $msbuild" -Level Success
                    return $msbuild
                }
            }
        }
    }
    
    # Fallback: Check common paths
    $commonPaths = @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2026\Enterprise\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2026\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            Write-BuildLog "Found MSBuild: $path" -Level Success
            return $path
        }
    }
    
    Write-BuildLog "MSBuild not found. Install Visual Studio 2022 Build Tools" -Level Error
    return $null
}

function Find-CMakePath {
    <#
    .SYNOPSIS
        Finds CMake executable path
    .OUTPUTS
        String path to cmake.exe or $null if not found
    #>
    [CmdletBinding()]
    param()
    
    # Check if cmake is in PATH
    $cmake = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if ($cmake) {
        Write-BuildLog "Found CMake: $($cmake.Source)" -Level Success
        return $cmake.Source
    }
    
    # Check common installation paths
    $commonPaths = @(
        "${env:ProgramFiles}\CMake\bin\cmake.exe",
        "${env:ProgramFiles(x86)}\CMake\bin\cmake.exe"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            Write-BuildLog "Found CMake: $path" -Level Success
            return $path
        }
    }
    
    Write-BuildLog "CMake not found. Install CMake 3.20+" -Level Error
    return $null
}

function Test-VisualStudioTools {
    <#
    .SYNOPSIS
        Verifies Visual Studio build tools are available
    .OUTPUTS
        Boolean indicating if tools are available
    #>
    [CmdletBinding()]
    param()
    
    $msbuild = Find-MSBuildPath
    $cmake = Find-CMakePath
    
    if (-not $msbuild -or -not $cmake) {
        Write-BuildLog "Required build tools not found" -Level Error
        return $false
    }
    
    Write-BuildLog "All required build tools found" -Level Success
    return $true
}

# ============================================================================
# Directory Management
# ============================================================================

function New-CleanDirectory {
    <#
    .SYNOPSIS
        Creates or cleans a directory
    .PARAMETER Path
        Directory path to create/clean
    .PARAMETER Clean
        If $true, removes existing directory before creating
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$Path,
        
        [Parameter()]
        [switch]$Clean
    )
    
    if ($Clean -and (Test-Path $Path)) {
        Write-BuildLog "Cleaning directory: $Path" -Level Info
        Remove-Item -Path $Path -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    if (-not (Test-Path $Path)) {
        Write-BuildLog "Creating directory: $Path" -Level Info
        New-Item -Path $Path -ItemType Directory -Force | Out-Null
    }
}

# ============================================================================
# CMake Build Functions
# ============================================================================

function Invoke-CMakeBuild {
    <#
    .SYNOPSIS
        Executes a CMake-based library build
    .PARAMETER LibraryName
        Name of the library (for logging)
    .PARAMETER SourceDir
        Path to source directory containing CMakeLists.txt
    .PARAMETER BuildDir
        Path to build directory (will be created)
    .PARAMETER InstallDir
        Optional install directory
    .PARAMETER Configuration
        Build configuration (Release or Debug)
    .PARAMETER CMakeOptions
        Additional CMake options as hashtable
    .PARAMETER Clean
        Clean build directory before building
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$LibraryName,
        
        [Parameter(Mandatory)]
        [string]$SourceDir,
        
        [Parameter(Mandatory)]
        [string]$BuildDir,
        
        [Parameter()]
        [string]$InstallDir,
        
        [Parameter()]
        [ValidateSet('Release', 'Debug')]
        [string]$Configuration = 'Release',
        
        [Parameter()]
        [hashtable]$CMakeOptions = @{},
        
        [Parameter()]
        [switch]$Clean
    )
    
    Write-BuildHeader "Building $LibraryName with CMake ($Configuration)"
    
    # Verify tools
    $cmake = Find-CMakePath
    if (-not $cmake) {
        throw "CMake not found"
    }
    
    # Verify source directory
    if (-not (Test-Path $SourceDir)) {
        throw "Source directory not found: $SourceDir"
    }
    
    $cmakeListsPath = Join-Path $SourceDir 'CMakeLists.txt'
    if (-not (Test-Path $cmakeListsPath)) {
        throw "CMakeLists.txt not found in: $SourceDir"
    }
    
    # Create/clean build directory
    New-CleanDirectory -Path $BuildDir -Clean:$Clean
    
    # Detect stale CMakeCache.txt (path mismatch from directory renames)
    $existingCache = Join-Path $BuildDir 'CMakeCache.txt'
    if (Test-Path $existingCache) {
        $cacheContent = Get-Content $existingCache -First 5 | Out-String
        $normalizedSource = $SourceDir.Replace('\', '/').TrimEnd('/')
        if ($cacheContent -notmatch [regex]::Escape($normalizedSource)) {
            Write-BuildLog "Stale CMakeCache.txt detected (directory was renamed). Cleaning build dir..." -Level Warning
            Remove-Item -Path $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
            New-CleanDirectory -Path $BuildDir
        }
    }
    
    # Build CMake arguments (use forward slashes for CMake path compatibility)
    $sourceDirCMake = $SourceDir.Replace('\', '/')
    $buildDirCMake = $BuildDir.Replace('\', '/')
    $cmakeArgs = @(
        "-S", "`"$sourceDirCMake`"",
        "-B", "`"$buildDirCMake`"",
        "-G", "`"$($Script:BuildConfig.CMakeGenerator)`"",
        "-A", "x64",
        "-T", $Script:BuildConfig.CMakeToolset
    )
    
    # Add install directory if specified
    if ($InstallDir) {
        $installDirCMake = $InstallDir.Replace('\', '/')
        $cmakeArgs += "-DCMAKE_INSTALL_PREFIX=`"$installDirCMake`""
    }
    
    # Add common options
    $cmakeArgs += "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded`$<`$<CONFIG:Debug>:Debug>DLL"
    
    # Add custom options (convert backslashes to forward slashes for CMake compatibility)
    foreach ($key in $CMakeOptions.Keys) {
        $value = [string]$CMakeOptions[$key]
        # CMake treats backslashes as escape sequences - always use forward slashes in paths
        $value = $value.Replace('\', '/')
        if ($value -match '[\s"]') {
            $escapedValue = $value.Replace('"', '`"')
            $cmakeArgs += "-D${key}=`"$escapedValue`""
        } else {
            $cmakeArgs += "-D${key}=$value"
        }
    }
    
    # Configure
    Write-BuildLog "Configuring with CMake..." -Level Info
    Write-BuildLog "CMake command: $cmake $($cmakeArgs -join ' ')" -Level Info
    
    $configureCmd = "& `"$cmake`" $($cmakeArgs -join ' ')"
    Invoke-Expression $configureCmd

    if ($LASTEXITCODE -ne 0) {
        $cachePath = Join-Path $BuildDir "CMakeCache.txt"
        if (Test-Path $cachePath) {
            Write-BuildLog "CMake configuration failed; stale cache/toolset mismatch suspected. Cleaning build dir and retrying once..." -Level Warning
            Remove-Item -Path $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
            New-CleanDirectory -Path $BuildDir
            Invoke-Expression $configureCmd
        }

        if ($LASTEXITCODE -ne 0) {
            Write-BuildLog "CMake configuration failed" -Level Error
            throw "CMake configuration failed with exit code $LASTEXITCODE"
        }
    }
    
    # Build
    Write-BuildLog "Building $Configuration configuration..." -Level Info
    
    $buildArgs = @(
        "--build", "`"$BuildDir`"",
        "--config", $Configuration,
        "--parallel", [Environment]::ProcessorCount
    )
    
    $buildCmd = "& `"$cmake`" $($buildArgs -join ' ')"
    Invoke-Expression $buildCmd
    
    if ($LASTEXITCODE -ne 0) {
        Write-BuildLog "Build failed" -Level Error
        throw "Build failed with exit code $LASTEXITCODE"
    }
    
    # Install (if install directory specified)
    if ($InstallDir) {
        Write-BuildLog "Installing to $InstallDir..." -Level Info
        
        $installArgs = @(
            "--install", "`"$BuildDir`"",
            "--config", $Configuration
        )
        
        $installCmd = "& `"$cmake`" $($installArgs -join ' ')"
        Invoke-Expression $installCmd
        
        if ($LASTEXITCODE -ne 0) {
            Write-BuildLog "Install failed" -Level Error
            throw "Install failed with exit code $LASTEXITCODE"
        }
    }
    
    Write-BuildLog "$LibraryName build completed successfully" -Level Success
}

# ============================================================================
# MSBuild Functions
# ============================================================================

function Invoke-MSBuildLibrary {
    <#
    .SYNOPSIS
        Executes an MSBuild-based library build
    .PARAMETER LibraryName
        Name of the library (for logging)
    .PARAMETER ProjectFile
        Path to .vcxproj or .sln file
    .PARAMETER Configuration
        Build configuration (Release or Debug)
    .PARAMETER Platform
        Build platform (x64)
    .PARAMETER Clean
        Perform clean before build
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$LibraryName,
        
        [Parameter(Mandatory)]
        [string]$ProjectFile,
        
        [Parameter()]
        [ValidateSet('Release', 'Debug')]
        [string]$Configuration = 'Release',
        
        [Parameter()]
        [string]$Platform = 'x64',
        
        [Parameter()]
        [switch]$Clean
    )
    
    Write-BuildHeader "Building $LibraryName with MSBuild ($Configuration)"
    
    # Verify tools
    $msbuild = Find-MSBuildPath
    if (-not $msbuild) {
        throw "MSBuild not found"
    }
    
    # Verify project file
    if (-not (Test-Path $ProjectFile)) {
        throw "Project file not found: $ProjectFile"
    }
    
    # Build MSBuild arguments
    $msbuildArgs = @(
        "`"$ProjectFile`"",
        "/p:Configuration=$Configuration",
        "/p:Platform=$Platform",
        "/m",  # Multi-processor build
        "/v:minimal"  # Minimal verbosity
    )
    
    if ($Clean) {
        $msbuildArgs += "/t:Clean"
    }
    
    $msbuildArgs += "/t:Build"
    
    # Execute MSBuild
    Write-BuildLog "Building with MSBuild..." -Level Info
    Write-BuildLog "MSBuild command: $msbuild $($msbuildArgs -join ' ')" -Level Info
    
    $buildCmd = "& `"$msbuild`" $($msbuildArgs -join ' ')"
    Invoke-Expression $buildCmd
    
    if ($LASTEXITCODE -ne 0) {
        Write-BuildLog "MSBuild failed" -Level Error
        throw "MSBuild failed with exit code $LASTEXITCODE"
    }
    
    Write-BuildLog "$LibraryName build completed successfully" -Level Success
}

# ============================================================================
# NMake Functions
# ============================================================================

function Invoke-NMakeBuild {
    <#
    .SYNOPSIS
        Executes an NMake-based library build
    .PARAMETER LibraryName
        Name of the library (for logging)
    .PARAMETER SourceDir
        Path to source directory containing Makefile
    .PARAMETER Target
        NMake target (default: all)
    .PARAMETER MakefileVars
        Makefile variables as hashtable
    .PARAMETER Clean
        Run clean target first
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$LibraryName,
        
        [Parameter(Mandatory)]
        [string]$SourceDir,
        
        [Parameter()]
        [string]$Target = 'all',
        
        [Parameter()]
        [string]$Makefile = '',
        
        [Parameter()]
        [hashtable]$MakefileVars = @{},
        
        [Parameter()]
        [switch]$Clean
    )
    
    Write-BuildHeader "Building $LibraryName with NMake"
    
    # Verify source directory
    if (-not (Test-Path $SourceDir)) {
        throw "Source directory not found: $SourceDir"
    }
    
    # Find nmake in PATH (should be in VS Developer Command Prompt)
    $nmake = Get-Command nmake.exe -ErrorAction SilentlyContinue
    if (-not $nmake) {
        # Try to find in VS installation
        $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        if (Test-Path $vswhere) {
            $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                -property installationPath
            
            if ($vsPath) {
                $vcvarsPath = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
                if (Test-Path $vcvarsPath) {
                    Write-BuildLog "Setting up Visual Studio environment..." -Level Info
                    cmd /c "`"$vcvarsPath`" && set" | ForEach-Object {
                        if ($_ -match '^([^=]+)=(.*)$') {
                            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
                        }
                    }
                }
            }
        }
        
        $nmake = Get-Command nmake.exe -ErrorAction SilentlyContinue
        if (-not $nmake) {
            throw "nmake.exe not found. Run from Visual Studio Developer Command Prompt"
        }
    }
    
    # Build nmake arguments
    $nmakeArgs = @()
    
    # Add /F <makefile> if specified
    if ($Makefile) {
        $nmakeArgs += "/F"
        $nmakeArgs += $Makefile
    }
    
    # Add makefile variables
    foreach ($key in $MakefileVars.Keys) {
        $value = $MakefileVars[$key]
        $nmakeArgs += "${key}=$value"
    }
    
    # Change to source directory
    Push-Location $SourceDir
    
    try {
        # Clean if requested
        if ($Clean) {
            Write-BuildLog "Cleaning..." -Level Info
            $cleanArgs = if ($Makefile) { @('/F', $Makefile, 'clean') } else { @('clean') }
            & nmake @cleanArgs 2>&1 | Out-Null
        }
        
        # Build
        Write-BuildLog "Building target: $Target" -Level Info
        $allArgs = $nmakeArgs + ($Target -split '\s+')
        Write-BuildLog "NMake command: nmake $($allArgs -join ' ')" -Level Info
        & nmake @allArgs
        
        if ($LASTEXITCODE -ne 0) {
            Write-BuildLog "NMake build failed" -Level Error
            throw "NMake failed with exit code $LASTEXITCODE"
        }
        
        Write-BuildLog "$LibraryName build completed successfully" -Level Success
    } finally {
        Pop-Location
    }
}

# ============================================================================
# Verification Functions
# ============================================================================

function Test-BuildOutput {
    <#
    .SYNOPSIS
        Verifies that build outputs exist
    .PARAMETER Files
        Array of expected output file paths
    .PARAMETER ThrowOnMissing
        Throw exception if files are missing
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string[]]$Files,
        
        [Parameter()]
        [switch]$ThrowOnMissing
    )
    
    Write-BuildLog "Verifying build outputs..." -Level Info
    
    $missingFiles = @()
    
    foreach ($file in $Files) {
        if (Test-Path $file) {
            $fileInfo = Get-Item $file
            $sizeKB = [math]::Round($fileInfo.Length / 1KB, 2)
            Write-BuildLog "  ✓ $file ($sizeKB KB)" -Level Success
        } else {
            Write-BuildLog "  ✗ $file (MISSING)" -Level Error
            $missingFiles += $file
        }
    }
    
    if ($missingFiles.Count -gt 0) {
        $message = "Missing $($missingFiles.Count) build output(s): $($missingFiles -join ', ')"
        if ($ThrowOnMissing) {
            throw $message
        } else {
            Write-BuildLog $message -Level Warning
            return $false
        }
    }
    
    Write-BuildLog "All build outputs verified" -Level Success
    return $true
}

function Copy-LibraryArtifacts {
    <#
    .SYNOPSIS
        Copies library artifacts to standard locations
    .PARAMETER SourceDir
        Directory containing build outputs
    .PARAMETER DestinationDir
        Destination directory for artifacts
    .PARAMETER Includes
        File patterns to include (e.g., '*.lib', '*.dll')
    #>
    [CmdletBinding()]
    param(
        [Parameter(Mandatory)]
        [string]$SourceDir,
        
        [Parameter(Mandatory)]
        [string]$DestinationDir,
        
        [Parameter()]
        [string[]]$Includes = @('*.lib', '*.dll', '*.pdb')
    )
    
    Write-BuildLog "Copying artifacts from $SourceDir to $DestinationDir..." -Level Info
    
    # Create destination directory
    New-CleanDirectory -Path $DestinationDir
    
    # Copy files
    foreach ($pattern in $Includes) {
        $files = Get-ChildItem -Path $SourceDir -Filter $pattern -Recurse -File
        foreach ($file in $files) {
            Copy-Item -Path $file.FullName -Destination $DestinationDir -Force
            Write-BuildLog "  Copied: $($file.Name)" -Level Info
        }
    }
    
    Write-BuildLog "Artifacts copied successfully" -Level Success
}

# ============================================================================
# Functions are automatically available when dot-sourced
# No Export-ModuleMember needed for script files
# ============================================================================

Write-BuildLog "Build-Library-Core.ps1 loaded successfully (v7.0)" -Level Success

