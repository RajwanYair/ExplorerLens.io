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
Set-Content $ciPath -Value $ci -NoNewline
Write-Host "[bump] copilot-instructions.md updated"

# 4. social-preview.svg
$svgPath = "$rootDir\docs\assets\social-preview.svg"
$svg = Get-Content $svgPath -Raw
$dateStr = Get-Date -Format "yyyy-MM-dd"
$svg = $svg -replace 'Last updated: v[\d.]+ "[^"]+" — \d{4}-\d{2}-\d{2}', "Last updated: v$Version `"$Codename`" — $dateStr"
$svg = $svg -replace 'font-size="15" fill="#c9d1d9">v[\d.]+ +"[^"]+"</text>', "font-size=`"15`" fill=`"#c9d1d9`">v$Version  `"$Codename`"</text>"
if ($TestCount -gt 0) {
    $svg = $svg -replace 'font-size="15" fill="#3fb950" font-weight="600">.[^\s]+ \d+ Tests</text>', "font-size=`"15`" fill=`"#3fb950`" font-weight=`"600`">&#x2713;  $TestCount Tests</text>"
}
Set-Content $svgPath -Value $svg -NoNewline
Write-Host "[bump] social-preview.svg updated"

# 5. CHANGELOG.md — prepend new section
$clPath = "$rootDir\CHANGELOG.md"
$cl = Get-Content $clPath -Raw
$marker = "## [Unreleased]"
$idx = $cl.IndexOf($marker)
if ($idx -ge 0) {
    $afterMarker = $cl.IndexOf("`n---", $idx)
    if ($afterMarker -ge 0) {
        $newSection = "`n`n$ChangelogEntry`n"
        $cl = $cl.Substring(0, $afterMarker + 5) + $newSection + $cl.Substring($afterMarker + 5)
        Set-Content $clPath -Value $cl -NoNewline
        Write-Host "[bump] CHANGELOG.md updated"
    }
}

# 6. Commit
$commitMsg = "chore: bump version to $Version ($Codename)"
$details = "Sprint version bump. VERSION, BuildValidation.h, CHANGELOG.md, copilot-instructions.md, social-preview.svg"
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
