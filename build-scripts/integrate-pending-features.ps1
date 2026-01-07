# Integrate Pending Features into DarkThumbs
# Automatically updates CBXShell.vcxproj to enable built libraries
# PowerShell script for automatic integration

param(
    [switch]$DryRun,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs - Feature Integration" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Setup paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$VcxprojPath = Join-Path $RootDir "CBXShell\CBXShell.vcxproj"
$ExternalDir = Join-Path $RootDir "external"

# Check which libraries are available
$LibjxlLib = Join-Path $ExternalDir "image-libs\libjxl-0.11.1\x64\Release\libjxl.lib"
$LibarchiveLib = Join-Path $ExternalDir "compression\libarchive-3.7.6\x64\Release\libarchive.lib"

$LibjxlAvailable = Test-Path $LibjxlLib
$LibarchiveAvailable = Test-Path $LibarchiveLib

Write-Host "Library Status Check:" -ForegroundColor Yellow
Write-Host "  - libjxl.lib: $(if($LibjxlAvailable){'FOUND'}else{'NOT FOUND'})" -ForegroundColor $(if($LibjxlAvailable){'Green'}else{'Red'})
Write-Host "  - libarchive.lib: $(if($LibarchiveAvailable){'FOUND'}else{'NOT FOUND'})" -ForegroundColor $(if($LibarchiveAvailable){'Green'}else{'Red'})
Write-Host ""

if (-not $LibjxlAvailable -and -not $LibarchiveAvailable) {
    Write-Host "[ERROR] No libraries found to integrate" -ForegroundColor Red
    Write-Host "Please run build-pending-features.ps1 first" -ForegroundColor Yellow
    exit 1
}

# Verify vcxproj exists
if (-not (Test-Path $VcxprojPath)) {
    Write-Host "[ERROR] CBXShell.vcxproj not found at: $VcxprojPath" -ForegroundColor Red
    exit 1
}

# Read vcxproj content
Write-Host "Reading CBXShell.vcxproj..." -ForegroundColor Yellow
$VcxprojContent = Get-Content $VcxprojPath -Raw

# Backup original file
if (-not $DryRun) {
    $BackupPath = "$VcxprojPath.backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
    Copy-Item $VcxprojPath $BackupPath
    Write-Host "[OK] Created backup: $(Split-Path $BackupPath -Leaf)" -ForegroundColor Green
}

$ChangesMade = $false

# Integrate libjxl
if ($LibjxlAvailable) {
    Write-Host ""
    Write-Host "Integrating JPEG XL Support (libjxl)..." -ForegroundColor Yellow
    
    # 1. Enable preprocessor definition
    if ($VcxprojContent -match '<!--\s*ENABLE_JXL_SUPPORT\s*;?\s*-->') {
        Write-Host "  [1/4] Enabling ENABLE_JXL_SUPPORT preprocessor definition..." -ForegroundColor White
        $VcxprojContent = $VcxprojContent -replace '<!--\s*(ENABLE_JXL_SUPPORT)\s*;?\s*-->', ';$1'
        $ChangesMade = $true
        Write-Host "        [OK] Preprocessor definition enabled" -ForegroundColor Green
    }
    elseif ($VcxprojContent -match 'ENABLE_JXL_SUPPORT') {
        Write-Host "  [1/4] ENABLE_JXL_SUPPORT already enabled" -ForegroundColor Green
    }
    else {
        Write-Host "  [1/4] Adding ENABLE_JXL_SUPPORT to preprocessor definitions..." -ForegroundColor White
        # Add after ENABLE_RAW_SUPPORT
        $VcxprojContent = $VcxprojContent -replace '(ENABLE_RAW_SUPPORT)(;|\s)', '$1;ENABLE_JXL_SUPPORT$2'
        $ChangesMade = $true
        Write-Host "        [OK] Preprocessor definition added" -ForegroundColor Green
    }
    
    # 2. Enable source file compilation
    if ($VcxprojContent -match '<!--\s*<ClCompile Include="jxl_decoder\.cpp"\s*/>\s*-->') {
        Write-Host "  [2/4] Enabling jxl_decoder.cpp compilation..." -ForegroundColor White
        $VcxprojContent = $VcxprojContent -replace '<!--\s*(<ClCompile Include="jxl_decoder\.cpp"\s*/>)\s*-->', '$1'
        $ChangesMade = $true
        Write-Host "        [OK] Source file enabled" -ForegroundColor Green
    }
    elseif ($VcxprojContent -match '<ClCompile Include="jxl_decoder\.cpp"') {
        Write-Host "  [2/4] jxl_decoder.cpp already enabled" -ForegroundColor Green
    }
    else {
        Write-Host "  [2/4] [WARNING] jxl_decoder.cpp not found in vcxproj" -ForegroundColor Yellow
    }
    
    # 3. Add library path
    $LibjxlPath = "..\external\image-libs\libjxl-0.11.1\x64\Release"
    if ($VcxprojContent -notmatch [regex]::Escape($LibjxlPath)) {
        Write-Host "  [3/4] Adding libjxl library path..." -ForegroundColor White
        # Find AdditionalLibraryDirectories and add path
        $VcxprojContent = $VcxprojContent -replace '(<AdditionalLibraryDirectories>.*?)(</AdditionalLibraryDirectories>)', "`$1;$LibjxlPath`$2"
        $ChangesMade = $true
        Write-Host "        [OK] Library path added" -ForegroundColor Green
    }
    else {
        Write-Host "  [3/4] libjxl library path already present" -ForegroundColor Green
    }
    
    # 4. Add library dependency
    if ($VcxprojContent -notmatch 'libjxl\.lib') {
        Write-Host "  [4/4] Adding libjxl.lib dependency..." -ForegroundColor White
        # Add to AdditionalDependencies after libsharpyuv.lib
        $VcxprojContent = $VcxprojContent -replace '(libsharpyuv\.lib)(;|\s)', '$1;libjxl.lib$2'
        $ChangesMade = $true
        Write-Host "        [OK] Library dependency added" -ForegroundColor Green
    }
    else {
        Write-Host "  [4/4] libjxl.lib already in dependencies" -ForegroundColor Green
    }
    
    Write-Host "[OK] JPEG XL integration complete" -ForegroundColor Green
}

# Integrate libarchive
if ($LibarchiveAvailable) {
    Write-Host ""
    Write-Host "Integrating Archive Format Support (libarchive)..." -ForegroundColor Yellow
    
    # 1. Enable preprocessor definition
    if ($VcxprojContent -match '<!--\s*ENABLE_LIBARCHIVE_SUPPORT\s*;?\s*-->') {
        Write-Host "  [1/4] Enabling ENABLE_LIBARCHIVE_SUPPORT preprocessor definition..." -ForegroundColor White
        $VcxprojContent = $VcxprojContent -replace '<!--\s*(ENABLE_LIBARCHIVE_SUPPORT)\s*;?\s*-->', ';$1'
        $ChangesMade = $true
        Write-Host "        [OK] Preprocessor definition enabled" -ForegroundColor Green
    }
    elseif ($VcxprojContent -match 'ENABLE_LIBARCHIVE_SUPPORT') {
        Write-Host "  [1/4] ENABLE_LIBARCHIVE_SUPPORT already enabled" -ForegroundColor Green
    }
    else {
        Write-Host "  [1/4] Adding ENABLE_LIBARCHIVE_SUPPORT to preprocessor definitions..." -ForegroundColor White
        # Add after ENABLE_RAW_SUPPORT
        $VcxprojContent = $VcxprojContent -replace '(ENABLE_RAW_SUPPORT)(;|\s)', '$1;ENABLE_LIBARCHIVE_SUPPORT$2'
        $ChangesMade = $true
        Write-Host "        [OK] Preprocessor definition added" -ForegroundColor Green
    }
    
    # 2. Enable source file compilation
    if ($VcxprojContent -match '<!--\s*<ClCompile Include="libarchive_wrapper\.cpp"\s*/>\s*-->') {
        Write-Host "  [2/4] Enabling libarchive_wrapper.cpp compilation..." -ForegroundColor White
        $VcxprojContent = $VcxprojContent -replace '<!--\s*(<ClCompile Include="libarchive_wrapper\.cpp"\s*/>)\s*-->', '$1'
        $ChangesMade = $true
        Write-Host "        [OK] Source file enabled" -ForegroundColor Green
    }
    elseif ($VcxprojContent -match '<ClCompile Include="libarchive_wrapper\.cpp"') {
        Write-Host "  [2/4] libarchive_wrapper.cpp already enabled" -ForegroundColor Green
    }
    else {
        Write-Host "  [2/4] [WARNING] libarchive_wrapper.cpp not found in vcxproj" -ForegroundColor Yellow
    }
    
    # 3. Add library path
    $LibarchivePath = "..\external\compression\libarchive-3.7.6\x64\Release"
    if ($VcxprojContent -notmatch [regex]::Escape($LibarchivePath)) {
        Write-Host "  [3/4] Adding libarchive library path..." -ForegroundColor White
        # Find AdditionalLibraryDirectories and add path
        $VcxprojContent = $VcxprojContent -replace '(<AdditionalLibraryDirectories>.*?)(</AdditionalLibraryDirectories>)', "`$1;$LibarchivePath`$2"
        $ChangesMade = $true
        Write-Host "        [OK] Library path added" -ForegroundColor Green
    }
    else {
        Write-Host "  [3/4] libarchive library path already present" -ForegroundColor Green
    }
    
    # 4. Add library dependency
    if ($VcxprojContent -notmatch 'libarchive\.lib') {
        Write-Host "  [4/4] Adding libarchive.lib dependency..." -ForegroundColor White
        # Add to AdditionalDependencies after lzma.lib
        $VcxprojContent = $VcxprojContent -replace '(lzma\.lib)(;|\s)', '$1;libarchive.lib$2'
        $ChangesMade = $true
        Write-Host "        [OK] Library dependency added" -ForegroundColor Green
    }
    else {
        Write-Host "  [4/4] libarchive.lib already in dependencies" -ForegroundColor Green
    }
    
    Write-Host "[OK] Archive format integration complete" -ForegroundColor Green
}

# Write changes
if ($ChangesMade -and -not $DryRun) {
    Write-Host ""
    Write-Host "Saving updated CBXShell.vcxproj..." -ForegroundColor Yellow
    $VcxprojContent | Set-Content $VcxprojPath -NoNewline
    Write-Host "[OK] Changes saved" -ForegroundColor Green
}
elseif ($DryRun) {
    Write-Host ""
    Write-Host "[DRY RUN] Changes would be made but not saved" -ForegroundColor Yellow
}
else {
    Write-Host ""
    Write-Host "[INFO] No changes needed - all features already integrated" -ForegroundColor Green
}

# Summary
Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Integration Summary" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

if ($LibjxlAvailable) {
    Write-Host "[[OK]] JPEG XL Support (libjxl)" -ForegroundColor Green
    Write-Host "    - Preprocessor: ENABLE_JXL_SUPPORT" -ForegroundColor White
    Write-Host "    - Source: jxl_decoder.cpp" -ForegroundColor White
    Write-Host "    - Library: libjxl.lib" -ForegroundColor White
    Write-Host "    - Formats: .jxl, .jxr" -ForegroundColor White
}

if ($LibarchiveAvailable) {
    Write-Host "[[OK]] Archive Format Support (libarchive)" -ForegroundColor Green
    Write-Host "    - Preprocessor: ENABLE_LIBARCHIVE_SUPPORT" -ForegroundColor White
    Write-Host "    - Source: libarchive_wrapper.cpp" -ForegroundColor White
    Write-Host "    - Library: libarchive.lib" -ForegroundColor White
    Write-Host "    - Formats: .tar, .tar.gz, .tar.bz2, .tar.xz, .cpio, .iso" -ForegroundColor White
}

Write-Host ""

if (-not $DryRun -and $ChangesMade) {
    Write-Host "Next Steps:" -ForegroundColor Yellow
    Write-Host "  1. Rebuild CBXShell solution:" -ForegroundColor White
    Write-Host "     msbuild CBXShell.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64" -ForegroundColor Cyan
    Write-Host "  2. Test new format support" -ForegroundColor White
    Write-Host "  3. Update documentation with new formats" -ForegroundColor White
    Write-Host ""
    Write-Host "Backup saved at:" -ForegroundColor Yellow
    Write-Host "  $BackupPath" -ForegroundColor White
}
