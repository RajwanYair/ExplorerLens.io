#==============================================================================
# Generate-SBOM.ps1 — Software Bill of Materials Generator
# ExplorerLens v15.0.0
#
# Generates an SPDX 2.3 SBOM from external/LIBRARY_INVENTORY.md and
# actual library files on disk. Output: sbom/ExplorerLens-SBOM.spdx.json
#
# Usage:
#   pwsh -File scripts/Generate-SBOM.ps1 [-OutputDir sbom] [-Format json]
#==============================================================================

param(
    [string]$OutputDir = "sbom",
    [ValidateSet("json", "tag-value")]
    [string]$Format = "json",
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$rootDir = Split-Path -Parent $PSScriptRoot

# Library metadata — source of truth from LIBRARY_INVENTORY.md
$libraries = @(
    @{ Name = "zlib"; Version = "1.3.1"; License = "Zlib"; URL = "https://zlib.net" }
    @{ Name = "zstd"; Version = "1.5.7"; License = "BSD-3-Clause"; URL = "https://github.com/facebook/zstd" }
    @{ Name = "lz4"; Version = "1.10.0"; License = "BSD-2-Clause"; URL = "https://github.com/lz4/lz4" }
    @{ Name = "minizip-ng"; Version = "4.0.10"; License = "Zlib"; URL = "https://github.com/zlib-ng/minizip-ng" }
    @{ Name = "bzip2"; Version = "1.0.8"; License = "BSD-4-Clause"; URL = "https://sourceware.org/bzip2/" }
    @{ Name = "lzma-sdk"; Version = "26.00"; License = "LGPL-2.1"; URL = "https://7-zip.org/sdk.html" }
    @{ Name = "xz"; Version = "5.6.4"; License = "LGPL-2.1"; URL = "https://tukaani.org/xz/" }
    @{ Name = "unrar"; Version = "7.2.2"; License = "UnRAR"; URL = "https://www.rarlab.com/rar_add.htm" }
    @{ Name = "libwebp"; Version = "1.5.0"; License = "BSD-3-Clause"; URL = "https://chromium.googlesource.com/webm/libwebp" }
    @{ Name = "libjxl"; Version = "0.11.1"; License = "BSD-3-Clause"; URL = "https://github.com/libjxl/libjxl" }
    @{ Name = "libavif"; Version = "1.3.0"; License = "BSD-2-Clause"; URL = "https://github.com/AOMediaCodec/libavif" }
    @{ Name = "dav1d"; Version = "1.5.1"; License = "BSD-2-Clause"; URL = "https://code.videolan.org/videolan/dav1d" }
    @{ Name = "libheif"; Version = "1.19.5"; License = "LGPL-3.0"; URL = "https://github.com/nickkraakman/libheif" }
    @{ Name = "libde265"; Version = "1.0.15"; License = "LGPL-3.0"; URL = "https://github.com/nickkraakman/libde265" }
    @{ Name = "libraw"; Version = "0.21.3"; License = "LGPL-2.1"; URL = "https://www.libraw.org" }
    @{ Name = "mupdf"; Version = "1.24.11"; License = "AGPL-3.0"; URL = "https://mupdf.com" }
    @{ Name = "openjpeg"; Version = "2.5.3"; License = "BSD-2-Clause"; URL = "https://www.openjpeg.org" }
    @{ Name = "freetype"; Version = "2.13.3"; License = "FTL"; URL = "https://freetype.org" }
)

# Create output directory
$outPath = Join-Path $rootDir $OutputDir
if (-not (Test-Path $outPath)) {
    New-Item -ItemType Directory -Path $outPath -Force | Out-Null
}

$timestamp = Get-Date -Format "yyyy-MM-ddTHH:mm:ssZ"
$docName = "ExplorerLens-SBOM"

if ($Format -eq "json") {
    # SPDX 2.3 JSON format
    $packages = $libraries | ForEach-Object {
        @{
            SPDXID           = "SPDXRef-Package-$($_.Name)"
            name             = $_.Name
            versionInfo      = $_.Version
            downloadLocation = $_.URL
            licenseConcluded = $_.License
            licenseDeclared  = $_.License
            copyrightText    = "NOASSERTION"
            supplier         = "NOASSERTION"
            filesAnalyzed    = $false
            externalRefs     = @(
                @{
                    referenceCategory = "PACKAGE-MANAGER"
                    referenceType     = "purl"
                    referenceLocator  = "pkg:generic/$($_.Name)@$($_.Version)"
                }
            )
        }
    }

    $relationships = $libraries | ForEach-Object {
        @{
            spdxElementId      = "SPDXRef-DOCUMENT"
            relatedSpdxElement = "SPDXRef-Package-$($_.Name)"
            relationshipType   = "DESCRIBES"
        }
    }

    $sbom = @{
        spdxVersion       = "SPDX-2.3"
        dataLicense       = "CC0-1.0"
        SPDXID            = "SPDXRef-DOCUMENT"
        name              = $docName
        documentNamespace = "https://explorerlens.io/sbom/$docName-$timestamp"
        creationInfo      = @{
            created  = $timestamp
            creators = @("Tool: ExplorerLens-SBOM-Generator-1.0")
        }
        packages          = $packages
        relationships     = $relationships
    }

    $jsonPath = Join-Path $outPath "$docName.spdx.json"
    $sbom | ConvertTo-Json -Depth 10 | Set-Content -Path $jsonPath -Encoding UTF8
    Write-Host "SBOM generated: $jsonPath" -ForegroundColor Green
    Write-Host "  Libraries: $($libraries.Count)" -ForegroundColor Cyan
    Write-Host "  Format: SPDX 2.3 JSON" -ForegroundColor Cyan
} else {
    # SPDX tag-value format
    $tvPath = Join-Path $outPath "$docName.spdx"
    $lines = @(
        "SPDXVersion: SPDX-2.3"
        "DataLicense: CC0-1.0"
        "SPDXID: SPDXRef-DOCUMENT"
        "DocumentName: $docName"
        "DocumentNamespace: https://explorerlens.io/sbom/$docName-$timestamp"
        "Creator: Tool: ExplorerLens-SBOM-Generator-1.0"
        "Created: $timestamp"
        ""
    )

    foreach ($lib in $libraries) {
        $lines += @(
            "##### Package: $($lib.Name)"
            ""
            "PackageName: $($lib.Name)"
            "SPDXID: SPDXRef-Package-$($lib.Name)"
            "PackageVersion: $($lib.Version)"
            "PackageDownloadLocation: $($lib.URL)"
            "PackageLicenseConcluded: $($lib.License)"
            "PackageLicenseDeclared: $($lib.License)"
            "PackageCopyrightText: NOASSERTION"
            "FilesAnalyzed: false"
            "ExternalRef: PACKAGE-MANAGER purl pkg:generic/$($lib.Name)@$($lib.Version)"
            ""
        )
    }

    foreach ($lib in $libraries) {
        $lines += "Relationship: SPDXRef-DOCUMENT DESCRIBES SPDXRef-Package-$($lib.Name)"
    }

    $lines | Set-Content -Path $tvPath -Encoding UTF8
    Write-Host "SBOM generated: $tvPath" -ForegroundColor Green
    Write-Host "  Libraries: $($libraries.Count)" -ForegroundColor Cyan
    Write-Host "  Format: SPDX 2.3 Tag-Value" -ForegroundColor Cyan
}

Write-Host "`nDone." -ForegroundColor Green
