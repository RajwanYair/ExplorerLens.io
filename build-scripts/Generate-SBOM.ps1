<#
.SYNOPSIS
    Generate-SBOM.ps1 — Generate CycloneDX SBOM for ExplorerLens

.DESCRIPTION
    Generates a CycloneDX 1.6 Software Bill of Materials from the project's
    external library inventory. Updates docs/SBOM.json with current versions.

.PARAMETER OutputPath
    Output SBOM file path (default: docs/SBOM.json)

.EXAMPLE
    .\build-scripts\Generate-SBOM.ps1
#>

param(
    [string]$OutputPath = "docs/SBOM.json"
)

$ErrorActionPreference = "Stop"
$rootDir = Split-Path -Parent $PSScriptRoot

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "  SBOM Generation — ExplorerLens v15.0.0" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

$timestamp = Get-Date -Format "yyyy-MM-ddTHH:mm:ssZ"
$uuid = [System.Guid]::NewGuid().ToString()

# Define all external libraries
$libraries = @(
    @{ name = "zlib"; version = "1.3.1"; license = "Zlib"; purl = "pkg:generic/zlib@1.3.1" }
    @{ name = "LZ4"; version = "1.10.0"; license = "BSD-2-Clause"; purl = "pkg:generic/lz4@1.10.0" }
    @{ name = "zstd"; version = "1.5.7"; license = "BSD-3-Clause"; purl = "pkg:generic/zstd@1.5.7" }
    @{ name = "LZMA SDK"; version = "26.00"; license = "public-domain"; purl = "pkg:generic/lzma-sdk@26.00" }
    @{ name = "minizip-ng"; version = "4.0.10"; license = "Zlib"; purl = "pkg:generic/minizip-ng@4.0.10" }
    @{ name = "UnRAR"; version = "7.2.2"; license = "UnRAR"; purl = "pkg:generic/unrar@7.2.2" }
    @{ name = "libwebp"; version = "1.5.0"; license = "BSD-3-Clause"; purl = "pkg:generic/libwebp@1.5.0" }
    @{ name = "libavif"; version = "1.3.0"; license = "BSD-2-Clause"; purl = "pkg:generic/libavif@1.3.0" }
    @{ name = "libjxl"; version = "0.11.1"; license = "BSD-3-Clause"; purl = "pkg:generic/libjxl@0.11.1" }
    @{ name = "libheif"; version = "1.19.5"; license = "LGPL-3.0"; purl = "pkg:generic/libheif@1.19.5" }
    @{ name = "libde265"; version = "1.0.15"; license = "LGPL-3.0"; purl = "pkg:generic/libde265@1.0.15" }
    @{ name = "LibRaw"; version = "0.21.3"; license = "LGPL-2.1"; purl = "pkg:generic/libraw@0.21.3" }
    @{ name = "dav1d"; version = "1.5.1"; license = "BSD-2-Clause"; purl = "pkg:generic/dav1d@1.5.1" }
    @{ name = "OpenJPEG"; version = "2.5.3"; license = "BSD-2-Clause"; purl = "pkg:generic/openjpeg@2.5.3" }
    @{ name = "FreeType"; version = "2.13.3"; license = "FTL"; purl = "pkg:generic/freetype@2.13.3" }
    @{ name = "MuPDF"; version = "1.24.11"; license = "AGPL-3.0"; purl = "pkg:generic/mupdf@1.24.11" }
    @{ name = "bzip2"; version = "1.0.8"; license = "BSD-4-Clause"; purl = "pkg:generic/bzip2@1.0.8" }
)

# Build components array
$components = $libraries | ForEach-Object {
    @{
        type      = "library"
        "bom-ref" = $_.name.ToLower()
        name      = $_.name
        version   = $_.version
        licenses  = @(
            @{ license = @{ id = $_.license } }
        )
        purl      = $_.purl
        scope     = "required"
    }
}

# Build SBOM object
$sbom = [ordered]@{
    "`$schema"   = "https://cyclonedx.org/schema/bom-1.6.schema.json"
    bomFormat    = "CycloneDX"
    specVersion  = "1.6"
    version      = 1
    serialNumber = "urn:uuid:$uuid"
    metadata     = [ordered]@{
        timestamp = $timestamp
        tools     = @(
            @{
                vendor  = "ExplorerLens Project"
                name    = "Generate-SBOM.ps1"
                version = "2.0.0"
            }
        )
        component = [ordered]@{
            type        = "application"
            "bom-ref"   = "ExplorerLens-main"
            name        = "ExplorerLens"
            version     = "15.0.0"
            description = "Advanced Windows Shell Extension for GPU-accelerated thumbnail generation supporting 200+ file formats"
            licenses    = @(
                @{ license = @{ name = "MIT"; url = "https://opensource.org/licenses/MIT" } }
            )
            copyright   = "Copyright (c) 2024-2026 ExplorerLens Project"
        }
    }
    components   = $components
}

# Write to file
$outFile = Join-Path $rootDir $OutputPath
$sbom | ConvertTo-Json -Depth 10 | Set-Content -Path $outFile -Encoding UTF8

$libCount = $libraries.Count
Write-Host "`n[OK] SBOM generated with $libCount components" -ForegroundColor Green
Write-Host "     Output: $outFile" -ForegroundColor White
