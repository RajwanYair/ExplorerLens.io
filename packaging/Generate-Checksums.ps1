# Generate Checksums for ExplorerLens Release Packages
# Creates SHA256, SHA512, and MD5 checksums for distribution

param(
    [Parameter(Mandatory = $true)]
    [string]$Version,
    
    [string]$PackageDir = "$PSScriptRoot\..\packaging\release-packages",
    
    [switch]$IncludeMD5,
    [switch]$IncludeSHA512,
    [switch]$CreatePGP
)

$ErrorActionPreference = "Stop"

# ANSI colors
$Cyan = "`e[36m"
$Green = "`e[32m"
$Yellow = "`e[33m"
$Red = "`e[31m"
$Reset = "`e[0m"

function Write-Header {
    param([string]$Text)
    Write-Host "`n$Cyan═══════════════════════════════════════════════$Reset" -ForegroundColor Cyan
    Write-Host "$Cyan$Text$Reset" -ForegroundColor Cyan
    Write-Host "$Cyan═══════════════════════════════════════════════$Reset`n" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Text)
    Write-Host "  $Green✓$Reset $Text" -ForegroundColor Green
}

function Write-Info {
    param([string]$Text)
    Write-Host "  $Yellow→$Reset $Text" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Text)
    Write-Host "  $Red✗$Reset $Text" -ForegroundColor Red
}

function Get-FileHashMultiple {
    param(
        [string]$FilePath,
        [string[]]$Algorithms = @("SHA256")
    )
    
    $results = @{}
    foreach ($algo in $Algorithms) {
        try {
            $hash = Get-FileHash -Path $FilePath -Algorithm $algo
            $results[$algo] = $hash.Hash.ToLower()
        } catch {
            Write-Warning "Failed to compute $algo for $FilePath: $_"
            $results[$algo] = $null
        }
    }
    return $results
}

function Format-FileSize {
    param([long]$Bytes)
    
    if ($Bytes -ge 1GB) {
        return "{0:N2} GB" -f ($Bytes / 1GB)
    } elseif ($Bytes -ge 1MB) {
        return "{0:N2} MB" -f ($Bytes / 1MB)
    } elseif ($Bytes -ge 1KB) {
        return "{0:N2} KB" -f ($Bytes / 1KB)
    } else {
        return "$Bytes bytes"
    }
}

Write-Header "ExplorerLens Checksum Generator v1.0.0"

# Validate version format
if ($Version -notmatch '^\d+\.\d+\.\d+$') {
    Write-Error "Invalid version format. Expected: X.Y.Z (e.g., 7.0.0)"
    exit 1
}

Write-Info "Version: $Version"
Write-Info "Package Directory: $PackageDir"

# Resolve package directory
if (-not (Test-Path $PackageDir)) {
    Write-Error "Package directory not found: $PackageDir"
    Write-Info "Please build release packages first or specify correct directory with -PackageDir"
    exit 1
}

$PackageDir = Resolve-Path $PackageDir

# Find release packages
Write-Header "Scanning for Release Packages"

$packages = @(
    "ExplorerLens-Setup-$Version.msi",
    "ExplorerLens-v$Version-x64-Setup.exe",
    "ExplorerLens-v$Version-Portable-x64.zip"
)

$foundFiles = @()

foreach ($package in $packages) {
    $fullPath = Join-Path $PackageDir $package
    if (Test-Path $fullPath) {
        $fileInfo = Get-Item $fullPath
        $foundFiles += $fileInfo
        Write-Success "$package ($(Format-FileSize $fileInfo.Length))"
    } else {
        Write-Warning "Package not found: $package"
    }
}

if ($foundFiles.Count -eq 0) {
    Write-Error "No release packages found in $PackageDir"
    Write-Info "Expected files:"
    foreach ($pkg in $packages) {
        Write-Info "  - $pkg"
    }
    exit 1
}

Write-Info "Found $($foundFiles.Count) package(s)"

# Determine hash algorithms
$algorithms = @("SHA256")
if ($IncludeSHA512) {
    $algorithms += "SHA512"
}
if ($IncludeMD5) {
    $algorithms += "MD5"
}

Write-Header "Computing Checksums"
Write-Info "Algorithms: $($algorithms -join ', ')"

# Compute hashes for all files
$allHashes = @{}

foreach ($file in $foundFiles) {
    Write-Host "`n$Yellow→ Processing:$Reset $($file.Name)" -ForegroundColor Yellow
    
    $hashes = Get-FileHashMultiple -FilePath $file.FullName -Algorithms $algorithms
    $allHashes[$file.Name] = $hashes
    
    foreach ($algo in $algorithms) {
        if ($hashes[$algo]) {
            Write-Info "$algo : $($hashes[$algo])"
        }
    }
}

# Create checksum files
Write-Header "Generating Checksum Files"

# SHA256SUMS (primary, Unix-style format)
$sha256File = Join-Path $PackageDir "SHA256SUMS"
$sha256Content = @()
$sha256Content += "# ExplorerLens v$Version - SHA256 Checksums"
$sha256Content += "# Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss UTC')"
$sha256Content += "# "
$sha256Content += "# Verify with: Get-FileHash <filename> -Algorithm SHA256"
$sha256Content += "# Or: sha256sum -c SHA256SUMS"
$sha256Content += ""

foreach ($file in $foundFiles) {
    $hash = $allHashes[$file.Name]["SHA256"]
    $sha256Content += "$hash  $($file.Name)"
}

$sha256Content | Out-File -FilePath $sha256File -Encoding UTF8
Write-Success "Created: SHA256SUMS"

# CHECKSUMS.txt (Windows-friendly format with all algorithms)
$checksumFile = Join-Path $PackageDir "CHECKSUMS.txt"
$checksumContent = @()
$checksumContent += "ExplorerLens v$Version - Package Checksums"
$checksumContent += "=" * 80
$checksumContent += "Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
$checksumContent += ""
$checksumContent += "Verification Instructions:"
$checksumContent += "  PowerShell: (Get-FileHash <filename> -Algorithm SHA256).Hash"
$checksumContent += "  Command: certutil -hashfile <filename> SHA256"
$checksumContent += ""
$checksumContent += "=" * 80
$checksumContent += ""

foreach ($file in $foundFiles) {
    $checksumContent += "File: $($file.Name)"
    $checksumContent += "Size: $(Format-FileSize $file.Length) ($($file.Length) bytes)"
    
    foreach ($algo in $algorithms) {
        $hash = $allHashes[$file.Name][$algo]
        if ($hash) {
            $checksumContent += "$algo : $hash"
        }
    }
    
    $checksumContent += ""
}

$checksumContent += "=" * 80
$checksumContent += "End of Checksums"
$checksumContent += ""

$checksumContent | Out-File -FilePath $checksumFile -Encoding UTF8
Write-Success "Created: CHECKSUMS.txt"

# Create individual .sha256 files for each package
foreach ($file in $foundFiles) {
    $hash = $allHashes[$file.Name]["SHA256"]
    $sha256SingleFile = "$($file.FullName).sha256"
    "$hash  $($file.Name)" | Out-File -FilePath $sha256SingleFile -Encoding ASCII -NoNewline
    Write-Success "Created: $($file.Name).sha256"
}

# Optional: SHA512SUMS
if ($IncludeSHA512) {
    $sha512File = Join-Path $PackageDir "SHA512SUMS"
    $sha512Content = @()
    $sha512Content += "# ExplorerLens v$Version - SHA512 Checksums"
    $sha512Content += "# Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss UTC')"
    $sha512Content += ""
    
    foreach ($file in $foundFiles) {
        $hash = $allHashes[$file.Name]["SHA512"]
        if ($hash) {
            $sha512Content += "$hash  $($file.Name)"
        }
    }
    
    $sha512Content | Out-File -FilePath $sha512File -Encoding UTF8
    Write-Success "Created: SHA512SUMS"
}

# Optional: MD5SUMS (legacy, not recommended for security)
if ($IncludeMD5) {
    $md5File = Join-Path $PackageDir "MD5SUMS"
    $md5Content = @()
    $md5Content += "# ExplorerLens v$Version - MD5 Checksums (legacy)"
    $md5Content += "# WARNING: MD5 is insecure. Use SHA256 for verification."
    $md5Content += ""
    
    foreach ($file in $foundFiles) {
        $hash = $allHashes[$file.Name]["MD5"]
        if ($hash) {
            $md5Content += "$hash  $($file.Name)"
        }
    }
    
    $md5Content | Out-File -FilePath $md5File -Encoding UTF8
    Write-Success "Created: MD5SUMS (legacy)"
}

# Optional: Create PGP signature
if ($CreatePGP) {
    Write-Header "Creating PGP Signatures"
    
    $gpg = Get-Command gpg -ErrorAction SilentlyContinue
    if ($gpg) {
        try {
            # Sign SHA256SUMS file
            & gpg --detach-sign --armor "$sha256File"
            if ($LASTEXITCODE -eq 0) {
                Write-Success "Created: SHA256SUMS.asc"
            } else {
                Write-Warning "PGP signing failed (exit code: $LASTEXITCODE)"
            }
        } catch {
            Write-Warning "PGP signing failed: $_"
        }
    } else {
        Write-Warning "GPG not found. Skipping PGP signatures."
        Write-Info "Install GPG from: https://www.gnupg.org/download/"
    }
}

# Summary
Write-Header "Summary"

Write-Info "Processed $($foundFiles.Count) package(s)"
Write-Info "Total size: $(Format-FileSize ($foundFiles | Measure-Object -Property Length -Sum).Sum)"

Write-Host "`n$Green✓ Checksum generation complete!$Reset`n" -ForegroundColor Green

Write-Host "Generated files:" -ForegroundColor Cyan
Write-Host "  - SHA256SUMS               (primary, Unix-style)" -ForegroundColor Gray
Write-Host "  - CHECKSUMS.txt            (detailed, Windows-friendly)" -ForegroundColor Gray
Write-Host "  - *.sha256                 (individual file checksums)" -ForegroundColor Gray

if ($IncludeSHA512) {
    Write-Host "  - SHA512SUMS               (additional security)" -ForegroundColor Gray
}

if ($IncludeMD5) {
    Write-Host "  - MD5SUMS                  (legacy compatibility)" -ForegroundColor Gray
}

if ($CreatePGP -and (Test-Path "$sha256File.asc")) {
    Write-Host "  - SHA256SUMS.asc           (PGP signature)" -ForegroundColor Gray
}

Write-Host "`nVerification example:" -ForegroundColor Cyan
Write-Host "  PowerShell:" -ForegroundColor Yellow
Write-Host "    `$expected = (Get-Content 'SHA256SUMS' | Select-String '$($foundFiles[0].Name)').Line.Split()[0]" -ForegroundColor Gray
Write-Host "    `$actual = (Get-FileHash '$($foundFiles[0].Name)').Hash.ToLower()" -ForegroundColor Gray
Write-Host "    if (`$actual -eq `$expected) { 'Checksum OK' } else { 'Checksum FAILED' }" -ForegroundColor Gray

Write-Host "`n  Linux/macOS:" -ForegroundColor Yellow
Write-Host "    sha256sum -c SHA256SUMS" -ForegroundColor Gray

Write-Host ""

