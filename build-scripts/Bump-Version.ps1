#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Bump-Version.ps1 — Automated version bump across all files
    Internal helper used by the sprint execution automation.

.PARAMETER Version
    New semantic version (e.g. "15.7.0")

.PARAMETER Codename
    Release codename (e.g. "Zenith-X")

.PARAMETER ChangelogEntry
    Multi-line string with the [Version] CHANGELOG section body.

.PARAMETER TagAndPush
    If set, create git tag and push after commit.

.PARAMETER TestCount
    Updated test count for SVG badge.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)] [string] $Version,
    [Parameter(Mandatory)] [string] $Codename,
    [Parameter(Mandatory)] [string] $ChangelogEntry,
    [int]    $TestCount  = 0,
    [switch] $TagAndPush
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir   = Split-Path -Parent $scriptDir
Set-Location $rootDir

$oldVersion = (Get-Content "$rootDir\VERSION" -Raw).Trim()
Write-Host "[bump] $oldVersion -> $Version ($Codename)"

# 1. VERSION file
Set-Content "$rootDir\VERSION" -Value $Version -NoNewline

# 2. BuildValidation.h
$bvPath = "$rootDir\Engine\Core\BuildValidation.h"
$bv = Get-Content $bvPath -Raw
$parts = $Version -split '\.'
$major = [int]$parts[0]; $minor = [int]$parts[1]; $patch = [int]$parts[2]

# Replace version constants using regex
$bv = $bv -replace 'static constexpr int MajorVersion = \d+;', "static constexpr int MajorVersion = $major;"
$bv = $bv -replace 'static constexpr int MinorVersion = \d+;', "static constexpr int MinorVersion = $minor;"
$bv = $bv -replace 'static constexpr int PatchVersion = \d+;', "static constexpr int PatchVersion = $patch;"
$bv = $bv -replace 'static constexpr const char\* VersionString = "[^"]+";', "static constexpr const char* VersionString = `"$Version`";"
$bv = $bv -replace 'static constexpr const char\* Codename = "[^"]+";', "static constexpr const char* Codename = `"$Codename`";"
Set-Content $bvPath -Value $bv -NoNewline
Write-Host "[bump] BuildValidation.h updated"

# 3. copilot-instructions.md
$ciPath = "$rootDir\.github\copilot-instructions.md"
$ci = Get-Content $ciPath -Raw
$ci = $ci -replace '(?m)^- \*\*Version:\*\* .+', "- **Version:** $Version (Codename: $Codename)"
$ci = $ci -replace '(?m)^- \*\*Current version:\*\* .+', "- **Current version:** v$Version `"$Codename`""
try { Set-Content $ciPath -Value $ci -NoNewline; Write-Host "[bump] copilot-instructions.md updated" }
catch { Write-Warning "[bump] copilot-instructions.md locked by VS Code — skipping (already at $Version)" }

# 4. social-preview.svg
$svgPath = "$rootDir\docs\assets\social-preview.svg"
$svg = Get-Content $svgPath -Raw
$dateStr = Get-Date -Format "yyyy-MM-dd"
$svg = $svg -replace 'Last updated: v[\d.]+ "[^"]+" — \d{4}-\d{2}-\d{2}', "Last updated: v$Version `"$Codename`" — $dateStr"
$svg = $svg -replace 'font-size="15" fill="#c9d1d9">v[\d.]+ +"[^"]+"</text>', "font-size=`"15`" fill=`"#c9d1d9`">v$Version  `"$Codename`"</text>"
if ($TestCount -gt 0) {
    $svg = $svg -replace 'font-size="15" fill="#3fb950" font-weight="600">[^<]+ Tests</text>', "font-size=`"15`" fill=`"#3fb950`" font-weight=`"600`">&#x2713;  $TestCount Tests</text>"
}
Set-Content $svgPath -Value $svg -NoNewline
Write-Host "[bump] social-preview.svg updated"

# 5a. Root CMakeLists.txt — project VERSION
$rootCmakePath = "$rootDir\CMakeLists.txt"
$rootCmake = Get-Content $rootCmakePath -Raw
$rootCmake = $rootCmake -replace '(# Version: )[\d.]+ "[^"]+"', "`${1}$Version `"$Codename`""
$rootCmake = $rootCmake -replace '(?m)(project\(ExplorerLens\s+VERSION\s+)[\d.]+', "`${1}$Version"
Set-Content $rootCmakePath -Value $rootCmake -NoNewline
Write-Host "[bump] CMakeLists.txt updated"

# 5b. Engine/CMakeLists.txt — project VERSION
$engCmakePath = "$rootDir\Engine\CMakeLists.txt"
$engCmake = Get-Content $engCmakePath -Raw
$engCmake = $engCmake -replace '(# Version: )[\d.]+', "`${1}$Version"
$engCmake = $engCmake -replace '(?m)(project\(ExplorerLensEngine\s+VERSION\s+)[\d.]+', "`${1}$Version"
Set-Content $engCmakePath -Value $engCmake -NoNewline
Write-Host "[bump] Engine/CMakeLists.txt updated"

# 5c. LENSManager/LENSManager.rc — VERSIONINFO
$rcPath = "$rootDir\LENSManager\LENSManager.rc"
$rc = Get-Content $rcPath -Raw
$rcVer = "$major, $minor, $patch, 0"
$rcVerStr = "$Version.0"
$rc = $rc -replace 'FILEVERSION\s+[\d, ]+', "FILEVERSION     $rcVer"
$rc = $rc -replace 'PRODUCTVERSION\s+[\d, ]+', "PRODUCTVERSION  $rcVer"
$rc = $rc -replace '(VALUE "FileVersion",\s*")[^"]*(")', "`${1}$rcVerStr\0`${2}"
$rc = $rc -replace '(VALUE "ProductVersion",\s*")[^"]*(")', "`${1}$rcVerStr\0`${2}"
Set-Content $rcPath -Value $rc -NoNewline
Write-Host "[bump] LENSManager.rc updated"

# 6. SBOMGenerator.h — two hardcoded version strings
$sbomGenPath = "$rootDir\Engine\Core\SBOMGenerator.h"
if (Test-Path $sbomGenPath) {
    $sbomGen = Get-Content $sbomGenPath -Raw
    $sbomGen = $sbomGen -replace 'ExplorerLens-[\d.]+', "ExplorerLens-$Version"
    $sbomGen = $sbomGen -replace 'ExplorerLens-SBOMGenerator-[\d.]+', "ExplorerLens-SBOMGenerator-$Version"
    Set-Content $sbomGenPath -Value $sbomGen -NoNewline
    Write-Host "[bump] SBOMGenerator.h updated"
}

# 7. vcpkg.json — "version" field
$vcpkgPath = "$rootDir\vcpkg.json"
if (Test-Path $vcpkgPath) {
    $vcpkg = Get-Content $vcpkgPath -Raw
    $vcpkg = $vcpkg -replace '("version":\s*")[\d.]+(")', "`${1}$Version`${2}"
    Set-Content $vcpkgPath -Value $vcpkg -NoNewline
    Write-Host "[bump] vcpkg.json updated"
}

# 8. baseline.json — _comment, version, _updated
$baselinePath = "$rootDir\Engine\Tests\benchmarks\baseline.json"
if (Test-Path $baselinePath) {
    $baseline = Get-Content $baselinePath -Raw
    $dateStr = Get-Date -Format "yyyy-MM-dd"
    $baseline = $baseline -replace '("_comment":\s*"ExplorerLens Engine Performance Baselines — v)[\d.]+ [^"]+(")', "`${1}$Version $Codename`${2}"
    $baseline = $baseline -replace '("version":\s*")[\d.]+(")', "`${1}$Version`${2}"
    $baseline = $baseline -replace '("_updated":\s*")[\d-]+(")', "`${1}$dateStr`${2}"
    Set-Content $baselinePath -Value $baseline -NoNewline
    Write-Host "[bump] baseline.json updated"
}

# 9. README.md — Tests badge + feature table row
$readmePath = "$rootDir\README.md"
if (Test-Path $readmePath) {
    $readme = Get-Content $readmePath -Raw
    if ($TestCount -gt 0) {
        $readme = $readme -replace 'Tests-\d+%20passing', "Tests-$TestCount%20passing"
        $readme = $readme -replace '(\| \*\*Tests\*\*\s*\| )[\d,]+ unit tests', "`${1}$("{0:N0}" -f $TestCount) unit tests"
    }
    Set-Content $readmePath -Value $readme -NoNewline
    Write-Host "[bump] README.md updated"
}

# 10. tool-versions.md — date + version in header
$tvPath = "$rootDir\.github\standards\tool-versions.md"
if (Test-Path $tvPath) {
    $dateStr = Get-Date -Format "d MMMM yyyy"
    $tv = Get-Content $tvPath -Raw
    $tv = $tv -replace '\*\*Last Updated:\*\* .+', "**Last Updated:** $dateStr (v$Version $Codename release)"
    $tv = $tv -replace '\*\*Version:\*\* [\d.]+ "[^"]+"', "**Version:** $Version `"$Codename`""
    Set-Content $tvPath -Value $tv -NoNewline
    Write-Host "[bump] tool-versions.md updated"
}

# 11. SBOM.json — serialNumber + metadata.component.version + timestamp
$sbomJsonPath = "$rootDir\docs\SBOM.json"
if (Test-Path $sbomJsonPath) {
    $dateStr = Get-Date -Format "yyyy-MM-dd"
    $sbomJson = Get-Content $sbomJsonPath -Raw
    $sbomJson = $sbomJson -replace '("serialNumber":\s*"urn:uuid:ExplorerLens-v)[\d.]+-[\d-]+(")', "`${1}$Version-$dateStr`${2}"
    $sbomJson = $sbomJson -replace '("timestamp":\s*")[\d-]+T[\d:]+Z(")', "`${1}${dateStr}T00:00:00Z`${2}"
    $sbomJson = $sbomJson -replace '("version":\s*")[\d.]+(",\s*\n\s*"description")', "`${1}$Version`${2}"
    Set-Content $sbomJsonPath -Value $sbomJson -NoNewline
    Write-Host "[bump] SBOM.json updated"
}

# 12. architecture-build.svg — MSI artifact filename chip
$archSvgPath = "$rootDir\docs\assets\architecture-build.svg"
if (Test-Path $archSvgPath) {
    $archSvg = Get-Content $archSvgPath -Raw
    $archSvg = $archSvg -replace 'ExplorerLens-[\d.]+-x64\.msi', "ExplorerLens-$Version-x64.msi"
    $archSvg = $archSvg -replace '(v)[\d.]+ "[^"]+"', "`${1}$Version `"$Codename`""
    Set-Content $archSvgPath -Value $archSvg -NoNewline
    Write-Host "[bump] architecture-build.svg updated"
}

# 12c. build-method.md — version line
$bmPath = "$rootDir\.github\standards\build-method.md"
if (Test-Path $bmPath) {
    $bm = Get-Content $bmPath -Raw
    $bm = $bm -replace '\*\*Version:\*\* [\d.]+ \(Codename: [^)]+\)', "**Version:** $Version (Codename: $Codename)"
    Set-Content $bmPath -Value $bm -NoNewline
    Write-Host "[bump] build-method.md updated"
}

# 13b. copilot-instructions.md — test count line
if ($TestCount -gt 0) {
    $ci2 = Get-Content $ciPath -Raw
    $ci2 = $ci2 -replace '~\d+ unit tests', "~$TestCount unit tests"
    $ci2 = $ci2 -replace '\(v[\d.]+ baseline\)', "(v$Version baseline)"
    try { Set-Content $ciPath -Value $ci2 -NoNewline; Write-Host "[bump] copilot-instructions.md test count updated" }
    catch { Write-Warning "[bump] copilot-instructions.md locked — test count skip" }
}

# 14. Packaging manifests — always in sync with main version

# 14a. packaging/npm/package.json — "version" field
$npmPath = "$rootDir\packaging\npm\package.json"
if (Test-Path $npmPath) {
    $npm = Get-Content $npmPath -Raw
    $npm = $npm -replace '("version":\s*")[\d.]+(")', "`${1}$Version`${2}"
    Set-Content $npmPath -Value $npm -NoNewline
    Write-Host "[bump] packaging/npm/package.json updated"
}

# 14b. packaging/ruby VERSION constant
$rubyPath = "$rootDir\packaging\ruby\lib\explorerlens\version.rb"
if (Test-Path $rubyPath) {
    $ruby = Get-Content $rubyPath -Raw
    $ruby = $ruby -replace "(VERSION = ')[\d.]+(')", "`${1}$Version`${2}"
    Set-Content $rubyPath -Value $ruby -NoNewline
    Write-Host "[bump] packaging/ruby/lib/explorerlens/version.rb updated"
}

# 14c. Dockerfile — ARG EXPLORERLENS_VERSION default
$dockerPath = "$rootDir\Dockerfile"
if (Test-Path $dockerPath) {
    $docker = Get-Content $dockerPath -Raw
    $docker = $docker -replace '(ARG EXPLORERLENS_VERSION=)[\d.]+', "`${1}$Version"
    Set-Content $dockerPath -Value $docker -NoNewline
    Write-Host "[bump] Dockerfile ARG EXPLORERLENS_VERSION updated"
}

# 15. CHANGELOG.md — prepend new versioned section (proper Keep-a-Changelog format)
$clPath = "$rootDir\CHANGELOG.md"
$cl = Get-Content $clPath -Raw
$marker = "## [Unreleased]"
$idx = $cl.IndexOf($marker)
if ($idx -ge 0) {
    $afterMarker = $cl.IndexOf("`n---", $idx)
    if ($afterMarker -ge 0) {
        $today = (Get-Date).ToString("yyyy-MM-dd")
        $sectionHeader = "## [$Version] — $today — $Codename"
        # Build a proper versioned section with a separator after it
        $newSection = "`n`n$sectionHeader`n`n$ChangelogEntry`n`n---"
        # Insert after the [Unreleased] separator, replacing nothing
        $cl = $cl.Substring(0, $afterMarker + 4) + $newSection + $cl.Substring($afterMarker + 4)
        Set-Content $clPath -Value $cl -NoNewline
        Write-Host "[bump] CHANGELOG.md updated — added $sectionHeader"
    }
}

# 6. Commit
$commitMsg = "chore: bump version to $Version ($Codename)"
$details = "Sprint version bump. All version-bearing files: VERSION, CMakeLists.txt, Engine/CMakeLists.txt, LENSManager.rc, BuildValidation.h, CHANGELOG.md, copilot-instructions.md, social-preview.svg, SBOMGenerator.h, vcpkg.json, baseline.json, README.md, tool-versions.md, SBOM.json, architecture-build.svg, build-method.md, packaging/npm/package.json, packaging/ruby/lib/explorerlens/version.rb, Dockerfile"
$fullMsg = "$commitMsg`n`n$details"
[IO.File]::WriteAllText("$rootDir\.git\BUMP_MSG.txt", $fullMsg)
git add -A
git commit -F "$rootDir\.git\BUMP_MSG.txt"
Remove-Item "$rootDir\.git\BUMP_MSG.txt" -ErrorAction SilentlyContinue
Write-Host "[bump] Committed: $commitMsg"

# 7. Optional tag + push
if ($TagAndPush) {
    git tag "v$Version"
    git push origin main --tags 2>&1
    Write-Host "[bump] Tagged v$Version and pushed"
}
