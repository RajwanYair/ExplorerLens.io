# Validate-Build.ps1
# Validates build integrity and runs comprehensive checks

param(
    [switch]$Rebuild = $false,
    [switch]$RunTests = $false,
    [switch]$Detailed = $false
)

$ErrorActionPreference = "Stop"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "ExplorerLens Build Validation" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$script:Errors = @()
$script:Warnings = @()
$script:StartTime = Get-Date

function Write-Success {
    param([string]$Message)
    Write-Host "✓ " -ForegroundColor Green -NoNewline
    Write-Host $Message
}

function Write-Error {
    param([string]$Message)
    Write-Host "✗ " -ForegroundColor Red -NoNewline
    Write-Host $Message
    $script:Errors += $Message
}

function Write-Warning {
    param([string]$Message)
    Write-Host "⚠ " -ForegroundColor Yellow -NoNewline
    Write-Host $Message
    $script:Warnings += $Message
}

function Test-FileExists {
    param(
        [string]$Path,
        [string]$Description
    )
    
    if (Test-Path $Path) {
        $size = (Get-Item $Path).Length
        $sizeMB = [math]::Round($size / 1MB, 2)
        Write-Success ("$Description exists (" + $sizeMB + " MB)")
        return $true
    } else {
        Write-Error ("$Description missing: " + $Path)
        return $false
    }
}

function Test-CompilationErrors {
    param([string]$LogPath)
    
    if (-not (Test-Path $LogPath)) {
        Write-Warning "Build log not found: $LogPath"
        return $true
    }
    
    $content = Get-Content $LogPath -Raw
    
    # Check for errors
    if ($content -match '(\d+) error\(s\)') {
        $errorCount = [int]$Matches[1]
        if ($errorCount -gt 0) {
            Write-Error "Build has $errorCount error(s)"
            return $false
        }
    }
    
    # Check for warnings
    if ($content -match '(\d+) warning\(s\)') {
        $warningCount = [int]$Matches[1]
        if ($warningCount -gt 0) {
            Write-Warning "Build has $warningCount warning(s)"
        }
    }
    
    Write-Success "No compilation errors found"
    return $true
}

function Test-Dependencies {
    Write-Host "`nChecking Dependencies..." -ForegroundColor Yellow
    
    $libs = @(
        "external\compression\zlib\lib\zlibstatic.lib",
        "external\compression\bzip2\lib\libbz2.lib",
        "external\compression\zstd\lib\libzstd_static.lib",
        "external\compression\lz4\lib\liblz4_static.lib",
        "external\compression\lzma\lib\lzma.lib",
        "external\compression\minizip-ng\lib\minizip.lib",
        "external\compression\unrar\lib\unrar.lib",
        "external\image\libwebp\lib\libwebp.lib"
    )
    
    $allExist = $true
    foreach ($lib in $libs) {
        if (Test-Path $lib) {
            if ($Detailed) {
                Write-Success "Found: $lib"
            }
        } else {
            Write-Error "Missing: $lib"
            $allExist = $false
        }
    }
    
    if ($allExist) {
        Write-Success ("All static libraries present (" + $libs.Count + " files)")
    }
    
    return $allExist
}

function Test-BuildOutputs {
    Write-Host "`nChecking Build Outputs..." -ForegroundColor Yellow
    
    $outputs = @{
        "LENSShell.dll" = "LENSShell\x64\Release\LENSShell.dll"
        "LENSManager.exe" = "LENSManager\x64\Release\LENSManager.exe"
    }
    
    $allExist = $true
    foreach ($output in $outputs.GetEnumerator()) {
        if (-not (Test-FileExists $output.Value $output.Key)) {
            $allExist = $false
        }
    }
    
    return $allExist
}

function Test-HeaderFiles {
    Write-Host "`nValidating Header Files..." -ForegroundColor Yellow
    
    $headers = @(
        "LENSShell\error_logger.h",
        "LENSShell\performance_profiler.h",
        "LENSShell\memory_utils.h",
        "LENSShell\enhanced_cache.h"
    )
    
    $allExist = $true
    foreach ($header in $headers) {
        if (Test-Path $header) {
            # Basic syntax check (look for obvious issues)
            $content = Get-Content $header -Raw
            
            if ($content -match '#pragma once' -or $content -match '#ifndef') {
                if ($Detailed) {
                    Write-Success "Header valid: $header"
                }
            } else {
                Write-Warning "Header may be missing include guard: $header"
            }
        } else {
            Write-Error "Header missing: $header"
            $allExist = $false
        }
    }
    
    if ($allExist) {
        Write-Success ("All new header files present (" + $headers.Count + " files)")
    }
    
    return $allExist
}

function Test-ProjectFiles {
    Write-Host "`nValidating Project Files..." -ForegroundColor Yellow
    
    $projects = @(
        "LENSShell\LENSShell.vcxproj",
        "LENSManager\LENSManager.vcxproj"
    )
    
    $allValid = $true
    foreach ($project in $projects) {
        if (Test-Path $project) {
            $xml = [xml](Get-Content $project)
            
            # Check for common issues
            $includes = $xml.SelectNodes("//ClInclude/@Include")
            $sources = $xml.SelectNodes("//ClCompile/@Include")
            
            if ($Detailed) {
                Write-Host "  Project: $project" -ForegroundColor Gray
                Write-Host "    Includes: $($includes.Count)" -ForegroundColor Gray
                Write-Host "    Sources: $($sources.Count)" -ForegroundColor Gray
            }
            
            Write-Success "Project file valid: $project"
        } else {
            Write-Error "Project file missing: $project"
            $allValid = $false
        }
    }
    
    return $allValid
}

function Invoke-Rebuild {
    Write-Host "`nRebuilding Project..." -ForegroundColor Yellow
    
    $msbuild = & "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsMSBuildCmd.bat" `
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" -ErrorAction SilentlyContinue
    
    if (-not $msbuild) {
        Write-Error "MSBuild not found"
        return $false
    }
    
    # Clean first
    Write-Host "  Cleaning..." -ForegroundColor Gray
    & $msbuild LENSShell\LENSShell.vcxproj /t:Clean /p:Configuration=Release /p:Platform=x64 /v:m /nologo
    & $msbuild LENSManager\LENSManager.vcxproj /t:Clean /p:Configuration=Release /p:Platform=x64 /v:m /nologo
    
    # Build LENSShell
    Write-Host "  Building LENSShell.dll..." -ForegroundColor Gray
    $result = & $msbuild LENSShell\LENSShell.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /v:m /nologo 2>&1
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "LENSShell build failed"
        Write-Host $result -ForegroundColor Red
        return $false
    }
    
    # Build LENSManager
    Write-Host "  Building LENSManager.exe..." -ForegroundColor Gray
    $result = & $msbuild LENSManager\LENSManager.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /v:m /nologo 2>&1
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "LENSManager build failed"
        Write-Host $result -ForegroundColor Red
        return $false
    }
    
    Write-Success "Build completed successfully"
    return $true
}

function Invoke-Tests {
    Write-Host "`nRunning Test Suite..." -ForegroundColor Yellow
    
    if (Test-Path ".\Test-ExplorerLens.ps1") {
        $result = & .\Test-ExplorerLens.ps1 -Quick
        return ($LASTEXITCODE -eq 0)
    } else {
        Write-Warning "Test suite not found"
        return $true
    }
}

# Main validation sequence
Write-Host "Starting validation at $(Get-Date -Format 'HH:mm:ss')`n"

$allPassed = $true

# 1. Check project files
if (-not (Test-ProjectFiles)) {
    $allPassed = $false
}

# 2. Check dependencies
if (-not (Test-Dependencies)) {
    $allPassed = $false
}

# 3. Check new headers
if (-not (Test-HeaderFiles)) {
    $allPassed = $false
}

# 4. Rebuild if requested
if ($Rebuild) {
    if (-not (Invoke-Rebuild)) {
        $allPassed = $false
    }
}

# 5. Check build outputs
if (-not (Test-BuildOutputs)) {
    $allPassed = $false
}

# 6. Run tests if requested
if ($RunTests) {
    if (-not (Invoke-Tests)) {
        $allPassed = $false
    }
}

# Summary
$elapsed = (Get-Date) - $script:StartTime
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Validation Summary" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "Duration: $($elapsed.TotalSeconds.ToString('F2'))s"
Write-Host "Errors: $($script:Errors.Count)" -ForegroundColor $(if ($script:Errors.Count -eq 0) { "Green" } else { "Red" })
Write-Host "Warnings: $($script:Warnings.Count)" -ForegroundColor $(if ($script:Warnings.Count -eq 0) { "Green" } else { "Yellow" })

if ($script:Errors.Count -gt 0) {
    Write-Host "`nErrors:" -ForegroundColor Red
    foreach ($error in $script:Errors) {
        Write-Host "  - $error" -ForegroundColor Red
    }
}

if ($script:Warnings.Count -gt 0 -and $Detailed) {
    Write-Host "`nWarnings:" -ForegroundColor Yellow
    foreach ($warning in $script:Warnings) {
        Write-Host "  - $warning" -ForegroundColor Yellow
    }
}

Write-Host ""

if ($allPassed -and $script:Errors.Count -eq 0) {
    Write-Host "✓ ALL VALIDATIONS PASSED" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ VALIDATION FAILED" -ForegroundColor Red
    exit 1
}

