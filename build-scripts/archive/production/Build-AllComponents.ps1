# DarkThumbs Complete Build Script
# PowerShell build automation for all components

param(
    [switch]$Clean,
    [switch]$Rebuild,
    [switch]$Parallel,
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    [ValidateSet('x64', 'Win32')]
    [string]$Platform = 'x64'
)

$ErrorActionPreference = "Stop"
$ProgressPreference = 'SilentlyContinue'

# Colors for output
function Write-BuildSuccess { param($msg) Write-Host "[OK] $msg" -ForegroundColor Green }
function Write-BuildInfo { param($msg) Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-BuildWarning { param($msg) Write-Host "[WARN] $msg" -ForegroundColor Yellow }
function Write-BuildError { param($msg) Write-Host "[FAIL] $msg" -ForegroundColor Red }

# Find MSBuild
function Find-MSBuild {
    $msbuildPaths = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2025\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    )
    
    foreach ($path in $msbuildPaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    throw "MSBuild not found. Please install Visual Studio Build Tools."
}

# Build configuration
$rootDir = Split-Path -Parent $PSScriptRoot
$msbuild = Find-MSBuild
$buildAction = if ($Rebuild) { "Rebuild" } elseif ($Clean) { "Clean" } else { "Build" }
$parallelFlag = if ($Parallel) { "/m" } else { "" }

Write-BuildInfo "DarkThumbs Build System"
Write-BuildInfo "======================="
Write-BuildInfo "Configuration: $Configuration"
Write-BuildInfo "Platform: $Platform"
Write-BuildInfo "Action: $buildAction"
Write-BuildInfo "MSBuild: $msbuild"
Write-BuildInfo ""

# Track build results
$buildResults = @()
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# Build compression libraries
function Build-CompressionLibs {
    Write-BuildInfo "Building Compression Libraries..."
    
    $libs = @(
        @{Name="zlib"; Path="external\compression\zlib\zlib.vcxproj"},
        @{Name="minizip-ng"; Path="external\compression\minizip-ng\minizip-ng.vcxproj"},
        @{Name="bzip2"; Path="external\compression\bzip2\bzip2.vcxproj"},
        @{Name="zstd"; Path="external\compression\zstd\zstd.vcxproj"},
        @{Name="lz4"; Path="external\compression\lz4\lz4.vcxproj"},
        @{Name="lzma"; Path="external\compression\lzma\lzma.vcxproj"},
        @{Name="unrar"; Path="external\compression\unrar\unrar.vcxproj"}
    )
    
    foreach ($lib in $libs) {
        $fullPath = Join-Path $rootDir $lib.Path
        if (Test-Path $fullPath) {
            try {
                Write-BuildInfo "  Building $($lib.Name)..."
                $output = & $msbuild $fullPath /t:$buildAction /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo $parallelFlag 2>&1
                
                if ($LASTEXITCODE -eq 0) {
                    Write-BuildSuccess "  $($lib.Name) built successfully"
                    $script:buildResults += @{Component=$lib.Name; Status="Success"; Time=$stopwatch.Elapsed}
                } else {
                    Write-BuildError "  $($lib.Name) build failed"
                    $script:buildResults += @{Component=$lib.Name; Status="Failed"; Time=$stopwatch.Elapsed}
                }
            } catch {
                Write-BuildError "  $($lib.Name) error: $_"
                $script:buildResults += @{Component=$lib.Name; Status="Error"; Time=$stopwatch.Elapsed}
            }
        } else {
            Write-BuildWarning "  $($lib.Name) project not found: $fullPath"
        }
    }
}

# Build image libraries
function Build-ImageLibs {
    Write-BuildInfo "Building Image Libraries..."
    
    $libs = @(
        @{Name="libwebp"; Path="external\image-libs\libwebp-1.5.0\libwebp.vcxproj"},
        @{Name="libsharpyuv"; Path="external\image-libs\libwebp-1.5.0\libsharpyuv.vcxproj"}
    )
    
    foreach ($lib in $libs) {
        $fullPath = Join-Path $rootDir $lib.Path
        if (Test-Path $fullPath) {
            try {
                Write-BuildInfo "  Building $($lib.Name)..."
                $output = & $msbuild $fullPath /t:$buildAction /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo $parallelFlag 2>&1
                
                if ($LASTEXITCODE -eq 0) {
                    Write-BuildSuccess "  $($lib.Name) built successfully"
                    $script:buildResults += @{Component=$lib.Name; Status="Success"; Time=$stopwatch.Elapsed}
                } else {
                    Write-BuildError "  $($lib.Name) build failed"
                    $script:buildResults += @{Component=$lib.Name; Status="Failed"; Time=$stopwatch.Elapsed}
                }
            } catch {
                Write-BuildError "  $($lib.Name) error: $_"
                $script:buildResults += @{Component=$lib.Name; Status="Error"; Time=$stopwatch.Elapsed}
            }
        } else {
            Write-BuildWarning "  $($lib.Name) project not found: $fullPath"
        }
    }
}

# Build main components
function Build-MainComponents {
    Write-BuildInfo "Building Main Components..."
    
    $components = @(
        @{Name="CBXShell"; Path="CBXShell\CBXShell.vcxproj"},
        @{Name="CBXManager"; Path="CBXManager\CBXManager.vcxproj"}
    )
    
    foreach ($comp in $components) {
        $fullPath = Join-Path $rootDir $comp.Path
        if (Test-Path $fullPath) {
            try {
                Write-BuildInfo "  Building $($comp.Name)..."
                $output = & $msbuild $fullPath /t:$buildAction /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo $parallelFlag 2>&1
                
                if ($LASTEXITCODE -eq 0) {
                    Write-BuildSuccess "  $($comp.Name) built successfully"
                    $script:buildResults += @{Component=$comp.Name; Status="Success"; Time=$stopwatch.Elapsed}
                    
                    # Get file size
                    if ($buildAction -eq "Build" -or $buildAction -eq "Rebuild") {
                        $outFile = Join-Path $rootDir "$($comp.Path -replace '\.vcxproj$', '')\$Platform\$Configuration\$($comp.Name).*"
                        $files = Get-ChildItem $outFile -ErrorAction SilentlyContinue
                        if ($files) {
                            $size = ($files | Select-Object -First 1).Length
                            Write-BuildInfo "    Size: $([math]::Round($size/1MB, 2)) MB"
                        }
                    }
                } else {
                    Write-BuildError "  $($comp.Name) build failed"
                    Write-Host $output
                    $script:buildResults += @{Component=$comp.Name; Status="Failed"; Time=$stopwatch.Elapsed}
                }
            } catch {
                Write-BuildError "  $($comp.Name) error: $_"
                $script:buildResults += @{Component=$comp.Name; Status="Error"; Time=$stopwatch.Elapsed}
            }
        } else {
            Write-BuildError "  $($comp.Name) project not found: $fullPath"
        }
    }
}

# Copy outputs to deployment directory
function Copy-Outputs {
    if ($buildAction -eq "Clean") { return }
    
    Write-BuildInfo "Copying outputs to deployment directory..."
    
    $deployDir = Join-Path $rootDir "$Platform\$Configuration"
    if (-not (Test-Path $deployDir)) {
        New-Item -ItemType Directory -Path $deployDir -Force | Out-Null
    }
    
    $files = @(
        @{Src="CBXShell\$Platform\$Configuration\CBXShell.dll"; Dst="CBXShell.dll"},
        @{Src="CBXManager\$Platform\$Configuration\CBXManager.exe"; Dst="CBXManager.exe"}
    )
    
    foreach ($file in $files) {
        $srcPath = Join-Path $rootDir $file.Src
        $dstPath = Join-Path $deployDir $file.Dst
        
        if (Test-Path $srcPath) {
            Copy-Item -Path $srcPath -Destination $dstPath -Force
            Write-BuildSuccess "  Copied $($file.Dst)"
        }
    }
}

# Main build sequence
try {
    # Build compression libraries
    Build-CompressionLibs
    Write-BuildInfo ""
    
    # Build image libraries
    Build-ImageLibs
    Write-BuildInfo ""
    
    # Build main components
    Build-MainComponents
    Write-BuildInfo ""
    
    # Copy outputs
    if ($buildAction -ne "Clean") {
        Copy-Outputs
        Write-BuildInfo ""
    }
    
    # Summary
    $stopwatch.Stop()
    Write-BuildInfo "Build Summary"
    Write-BuildInfo "============="
    
    $success = ($buildResults | Where-Object { $_.Status -eq "Success" }).Count
    $failed = ($buildResults | Where-Object { $_.Status -eq "Failed" -or $_.Status -eq "Error" }).Count
    
    Write-BuildInfo "Total components: $($buildResults.Count)"
    Write-BuildSuccess "Successful: $success"
    if ($failed -gt 0) {
        Write-BuildError "Failed: $failed"
    }
    Write-BuildInfo "Total time: $($stopwatch.Elapsed.ToString('mm\:ss'))"
    Write-BuildInfo ""
    
    if ($failed -eq 0) {
        Write-BuildSuccess "BUILD COMPLETED SUCCESSFULLY"
        exit 0
    } else {
        Write-BuildError "BUILD COMPLETED WITH ERRORS"
        exit 1
    }
    
} catch {
    Write-BuildError "Build script error: $_"
    exit 1
}
