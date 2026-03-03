# Build-PortableZip.ps1
# Creates portable ZIP distribution of ExplorerLens

param(
    [string]$Configuration = "Release",
    [string]$Version = "15.0.0",
    [string]$OutputDir = "",
    [switch]$IncludeDebugSymbols = $false,
    [switch]$IncludeSource = $false
)

$ErrorActionPreference = "Stop"

Write-Host "============================================" -ForegroundColor Cyan
Write-Host "ExplorerLens Portable ZIP Builder" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host "============================================`n" -ForegroundColor Cyan

$ScriptDir = Split-Path -Parent $PSCommandPath
$RootDir = Split-Path -Parent $ScriptDir

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = Join-Path $ScriptDir "output"
}

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    Write-Host "[Created] Output directory: $OutputDir" -ForegroundColor Green
}

# Create temporary staging directory
$StagingDir = Join-Path $env:TEMP "ExplorerLens-Staging-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
New-Item -ItemType Directory -Path $StagingDir -Force | Out-Null
Write-Host "[Created] Staging directory: $StagingDir" -ForegroundColor Green

try {
    # =============================================================================
    # 1. Copy main binaries
    # =============================================================================
    Write-Host "`n[1/6] Copying main binaries..." -ForegroundColor Yellow

    $BinariesDir = Join-Path $StagingDir "bin"
    New-Item -ItemType Directory -Path $BinariesDir -Force | Out-Null

    $MainBinaries = @(
        "x64\$Configuration\LENSShell.dll",
        "x64\$Configuration\LENSManager.exe"
    )

    foreach ($binary in $MainBinaries) {
        $sourcePath = Join-Path $RootDir $binary
        if (Test-Path $sourcePath) {
            $destPath = Join-Path $BinariesDir (Split-Path $binary -Leaf)
            Copy-Item $sourcePath $destPath -Force
            $sizeMB = [math]::Round((Get-Item $destPath).Length / 1MB, 2)
            Write-Host "  ✓ Copied $(Split-Path $binary -Leaf) ($sizeMB MB)" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Missing: $binary" -ForegroundColor Red
            throw "Required binary not found: $binary"
        }
    }

    # Copy debug symbols if requested
    if ($IncludeDebugSymbols) {
        Write-Host "`n[1b/6] Copying debug symbols..." -ForegroundColor Yellow
        foreach ($binary in $MainBinaries) {
            $pdbPath = $binary -replace '\.exe$|\.dll$', '.pdb'
            $sourcePdb = Join-Path $RootDir $pdbPath
            if (Test-Path $sourcePdb) {
                $destPdb = Join-Path $BinariesDir (Split-Path $pdbPath -Leaf)
                Copy-Item $sourcePdb $destPdb -Force
                Write-Host "  ✓ Copied $(Split-Path $pdbPath -Leaf)" -ForegroundColor Green
            }
        }
    }

    # =============================================================================
    # 2. Copy runtime dependencies
    # =============================================================================
    Write-Host "`n[2/6] Copying runtime dependencies..." -ForegroundColor Yellow

    $RuntimeDeps = @(
        "x64\$Configuration\libde265.dll",
        "x64\$Configuration\zlib1.dll"
    )

    foreach ($dep in $RuntimeDeps) {
        $sourcePath = Join-Path $RootDir $dep
        if (Test-Path $sourcePath) {
            $destPath = Join-Path $BinariesDir (Split-Path $dep -Leaf)
            Copy-Item $sourcePath $destPath -Force
            Write-Host "  ✓ Copied $(Split-Path $dep -Leaf)" -ForegroundColor Green
        } else {
            Write-Host "  ⚠ Optional dependency not found: $(Split-Path $dep -Leaf)" -ForegroundColor Yellow
        }
    }

    # =============================================================================
    # 3. Copy documentation
    # =============================================================================
    Write-Host "`n[3/6] Copying documentation..." -ForegroundColor Yellow

    $DocsDir = Join-Path $StagingDir "docs"
    New-Item -ItemType Directory -Path $DocsDir -Force | Out-Null

    $DocFiles = @(
        "README.md",
        "USER_GUIDE.md",
        "CHANGELOG.md",
        "LICENSE",
        "KNOWN_ISSUES.md"
    )

    foreach ($doc in $DocFiles) {
        $sourcePath = Join-Path $RootDir $doc
        if (Test-Path $sourcePath) {
            $destPath = Join-Path $DocsDir (Split-Path $doc -Leaf)
            Copy-Item $sourcePath $destPath -Force
            Write-Host "  ✓ Copied $doc" -ForegroundColor Green
        } else {
            Write-Host "  ⚠ Documentation not found: $doc" -ForegroundColor Yellow
        }
    }

    # Copy release notes if available
    $releaseNotesPath = Join-Path $RootDir "docs\release-notes\RELEASE_NOTES_v$Version.md"
    if (Test-Path $releaseNotesPath) {
        Copy-Item $releaseNotesPath (Join-Path $DocsDir "RELEASE_NOTES.md") -Force
        Write-Host "  ✓ Copied RELEASE_NOTES_v$Version.md" -ForegroundColor Green
    }

    # =============================================================================
    # 4. Create installation scripts
    # =============================================================================
    Write-Host "`n[4/6] Creating installation scripts..." -ForegroundColor Yellow

    $InstallScript = @'
# Install-ExplorerLens-Portable.ps1
# Portable installation script for ExplorerLens

param(
    [switch]$Uninstall = $false
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $PSCommandPath
$BinDir = Join-Path $ScriptDir "bin"

if ($Uninstall) {
    Write-Host "Unregistering ExplorerLens shell extension..." -ForegroundColor Yellow

    # Unregister COM server
    $regsvr32 = Join-Path $env:SystemRoot "System32\regsvr32.exe"
    $lensShellPath = Join-Path $BinDir "LENSShell.dll"

    if (Test-Path $lensShellPath) {
        & $regsvr32 /u /s $lensShellPath
        Write-Host "✓ Unregistered successfully" -ForegroundColor Green
    } else {
        Write-Host "✗ LENSShell.dll not found" -ForegroundColor Red
    }
} else {
    Write-Host "Installing ExplorerLens shell extension..." -ForegroundColor Yellow

    # Register COM server
    $regsvr32 = Join-Path $env:SystemRoot "System32\regsvr32.exe"
    $lensShellPath = Join-Path $BinDir "LENSShell.dll"

    if (Test-Path $lensShellPath) {
        & $regsvr32 /s $lensShellPath

        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ Registered successfully" -ForegroundColor Green
            Write-Host "`nExplorerLens is now installed!" -ForegroundColor Green
            Write-Host "Run LENSManager.exe to configure settings." -ForegroundColor Cyan
        } else {
            Write-Host "✗ Registration failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
            Write-Host "Try running as Administrator" -ForegroundColor Yellow
        }
    } else {
        Write-Host "✗ LENSShell.dll not found at: $lensShellPath" -ForegroundColor Red
    }
}
'@

    $installScriptPath = Join-Path $StagingDir "Install-ExplorerLens-Portable.ps1"
    Set-Content -Path $installScriptPath -Value $InstallScript -Force
    Write-Host "  ✓ Created Install-ExplorerLens-Portable.ps1" -ForegroundColor Green

    # Create README for portable version
    $PortableReadme = @"
ExplorerLens v$Version - Portable Edition
========================================

Quick Start:
1. Run 'Install-ExplorerLens-Portable.ps1' as Administrator to register the shell extension
2. Run 'bin\LENSManager.exe' to configure settings
3. Thumbnails will appear automatically in Windows Explorer

Uninstallation:
1. Run 'Install-ExplorerLens-Portable.ps1 -Uninstall' as Administrator
2. Delete this folder

Supported Formats:
- Archives: ZIP, RAR, 7Z, CBZ, CBR, EPUB
- Images: JPEG, PNG, WebP, AVIF, HEIF, HEIC, JXL, QOI, TGA, BMP, GIF, ICO, DDS, PSD, EXR
- RAW: CR2, NEF, ARW, DNG, ORF, RAF, RW2, and more
- 3D Models: OBJ, STL, GLTF, GLB
- Documents: PDF
- Media: MP3, MP4, MKV (album art/poster frames)
- Fonts: TTF, OTF
- Vector: SVG

For more information, see docs/README.md and docs/USER_GUIDE.md

Configuration Manager: bin\LENSManager.exe
Build Date: $(Get-Date -Format 'yyyy-MM-dd')
Version: $Version
"@

    $readmePath = Join-Path $StagingDir "README-PORTABLE.txt"
    Set-Content -Path $readmePath -Value $PortableReadme -Force
    Write-Host "  ✓ Created README-PORTABLE.txt" -ForegroundColor Green

    # =============================================================================
    # 5. Optional: Include source code
    # =============================================================================
    if ($IncludeSource) {
        Write-Host "`n[5/6] Including source code..." -ForegroundColor Yellow

        $SourceDir = Join-Path $StagingDir "src"
        New-Item -ItemType Directory -Path $SourceDir -Force | Out-Null

        # Copy essential source directories (exclude build artifacts)
        $SourceDirs = @("LENSShell", "LENSManager", "Engine", "SDK", "tests")
        foreach ($dir in $SourceDirs) {
            $srcPath = Join-Path $RootDir $dir
            if (Test-Path $srcPath) {
                $destPath = Join-Path $SourceDir $dir
                Copy-Item $srcPath $destPath -Recurse -Force -Exclude @("*.obj", "*.pdb", "*.ilk", "x64", "Debug", "Release")
                Write-Host "  ✓ Copied $dir/" -ForegroundColor Green
            }
        }

        # Copy solution file
        Copy-Item (Join-Path $RootDir "LENSShell.sln") (Join-Path $SourceDir "LENSShell.sln") -Force
        Write-Host "  ✓ Copied LENSShell.sln" -ForegroundColor Green
    }

    # =============================================================================
    # 6. Create ZIP archive
    # =============================================================================
    Write-Host "`n[6/6] Creating ZIP archive..." -ForegroundColor Yellow

    $ZipFileName = "ExplorerLens-$Version-Portable.zip"
    $ZipPath = Join-Path $OutputDir $ZipFileName

    # Remove old ZIP if exists
    if (Test-Path $ZipPath) {
        Remove-Item $ZipPath -Force
        Write-Host "  Removed old ZIP" -ForegroundColor Gray
    }

    # Create ZIP (requires PowerShell 5.0+)
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($StagingDir, $ZipPath, 'Optimal', $false)

    $zipSize = [math]::Round((Get-Item $ZipPath).Length / 1MB, 2)
    Write-Host "  ✓ Created $ZipFileName ($zipSize MB)" -ForegroundColor Green

    # =============================================================================
    # 7. Generate checksums
    # =============================================================================
    Write-Host "`n[7/6] Generating checksums..." -ForegroundColor Yellow

    $sha256 = (Get-FileHash -Path $ZipPath -Algorithm SHA256).Hash
    $checksumFile = Join-Path $OutputDir "ExplorerLens-$Version-Portable.sha256"

    $checksumContent = @"
$sha256  $ZipFileName

# Verification:
# PowerShell: (Get-FileHash -Path '$ZipFileName' -Algorithm SHA256).Hash
# Linux/Mac: sha256sum '$ZipFileName'
"@

    Set-Content -Path $checksumFile -Value $checksumContent -Force
    Write-Host "  ✓ SHA256: $sha256" -ForegroundColor Green
    Write-Host "  ✓ Saved to: $(Split-Path $checksumFile -Leaf)" -ForegroundColor Green

    # =============================================================================
    # SUCCESS
    # =============================================================================
    Write-Host "`n============================================" -ForegroundColor Green
    Write-Host "Portable ZIP Created Successfully!" -ForegroundColor Green
    Write-Host "============================================" -ForegroundColor Green
    Write-Host "  Output:   $ZipPath" -ForegroundColor Cyan
    Write-Host "  Size:     $zipSize MB" -ForegroundColor Cyan
    Write-Host "  SHA256:   $checksumFile" -ForegroundColor Cyan
    Write-Host "============================================`n" -ForegroundColor Green

} finally {
    # Clean up staging directory
    if (Test-Path $StagingDir) {
        Remove-Item $StagingDir -Recurse -Force
        Write-Host "[Cleaned] Temporary staging directory" -ForegroundColor Gray
    }
}
