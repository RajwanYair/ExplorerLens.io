#Requires -Version 7.0
<#
.SYNOPSIS
    Resolves GitHub Action tag references to their commit SHAs for security hardening.

.DESCRIPTION
    Scans .github/workflows/*.yml for `uses: owner/repo@vN` references and resolves
    them to the exact commit SHA at that tag using the GitHub API.

    Outputs a report showing:
    - All tag-based action refs found
    - The resolved SHA for each
    - A ready-to-use pinned replacement line

    Optionally writes the pinned SHAs back to the workflow files (with -Apply).

    WHY: Tag-based action references (e.g. @v4) can be changed by the action author
    after you've pinned them, allowing supply-chain attacks.  SHA-pinned references
    (e.g. @abc1234) are immutable.  See ROADMAP D40 / SLSA Supply Chain guidance.

.PARAMETER WorkflowDir
    Directory containing workflow YAML files.  Default: .github/workflows

.PARAMETER Token
    GitHub personal access token.  Defaults to $env:GITHUB_TOKEN.

.PARAMETER Apply
    Actually rewrite workflow files with pinned SHAs (default: dry-run only).

.PARAMETER Verbose
    Show individual API call progress.

.EXAMPLE
    # Dry run — show what would be pinned
    .\build-scripts\utilities\Pin-Actions.ps1

    # Actually pin all tag refs in workflow files
    .\build-scripts\utilities\Pin-Actions.ps1 -Apply

    # Use explicit token
    .\build-scripts\utilities\Pin-Actions.ps1 -Token $myPat -Apply

.NOTES
    Requires: gh CLI or $env:GITHUB_TOKEN set.
    Rate limit: 60 unauthenticated requests/hour; 5000 with token.
    Dependabot can keep SHAs current — see .github/dependabot.yml.
#>
[CmdletBinding(SupportsShouldProcess)]
param(
    [string]$WorkflowDir = '',
    [string]$Token       = $env:GITHUB_TOKEN,
    [switch]$Apply
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ── Locate workflows directory ────────────────────────────────────────────────
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
if (-not $WorkflowDir) {
    $WorkflowDir = Join-Path $repoRoot '.github\workflows'
}

if (-not (Test-Path $WorkflowDir)) {
    Write-Error "Workflows directory not found: $WorkflowDir"
    exit 1
}

# ── Build auth header ────────────────────────────────────────────────────────
$headers = @{ Accept = 'application/vnd.github.v3+json' }
if ($Token) {
    $headers['Authorization'] = "Bearer $Token"
    Write-Host "Using authenticated GitHub API (5000 req/hr limit)" -ForegroundColor Cyan
} else {
    Write-Warning "No GITHUB_TOKEN — using unauthenticated API (60 req/hr limit)"
}

# ── SHA resolution cache (avoid duplicate API calls) ─────────────────────────
$cache = @{}

function Resolve-ActionSha {
    param([string]$Owner, [string]$Repo, [string]$Ref)

    $key = "$Owner/$Repo@$Ref"
    if ($cache.ContainsKey($key)) { return $cache[$key] }

    $url = "https://api.github.com/repos/$Owner/$Repo/commits/$Ref"
    try {
        $resp = Invoke-RestMethod -Uri $url -Headers $headers -TimeoutSec 10
        $sha  = $resp.sha
        $cache[$key] = $sha
        return $sha
    }
    catch {
        Write-Warning "Failed to resolve $key : $_"
        return $null
    }
}

# ── Scan workflows ───────────────────────────────────────────────────────────
$pattern = [regex]'uses:\s+([a-zA-Z0-9_.-]+/[a-zA-Z0-9_.-]+)@(v\d+[^\s#]*)'
$workflows = Get-ChildItem $WorkflowDir -Filter '*.yml'

Write-Host "Scanning $($workflows.Count) workflow file(s) in $WorkflowDir" -ForegroundColor Cyan
Write-Host ""

$totalRefs = 0; $resolved = 0; $unresolved = 0
$allResults = [System.Collections.Generic.List[psobject]]::new()

foreach ($file in $workflows) {
    $content = Get-Content $file.FullName -Raw
    $matches  = $pattern.Matches($content)
    if ($matches.Count -eq 0) { continue }

    Write-Host "  $($file.Name) — $($matches.Count) tag ref(s)" -ForegroundColor White

    foreach ($m in $matches) {
        $totalRefs++
        $action = $m.Groups[1].Value
        $ref    = $m.Groups[2].Value
        $parts  = $action -split '/'

        $sha = Resolve-ActionSha -Owner $parts[0] -Repo $parts[1] -Ref $ref

        $result = [pscustomobject]@{
            File        = $file.Name
            Action      = $action
            TagRef      = $ref
            SHA         = $sha
            PinnedLine  = if ($sha) { "uses: $action@$sha  # $ref" } else { '<resolution failed>' }
            RawLine     = $m.Value
        }
        $allResults.Add($result)

        if ($sha) {
            $resolved++
            Write-Host "    $($action.PadRight(45)) $ref  →  $($sha.Substring(0,12))..." -ForegroundColor Green
        } else {
            $unresolved++
            Write-Host "    $($action.PadRight(45)) $ref  →  FAILED" -ForegroundColor Red
        }
    }
}

# ── Summary report ───────────────────────────────────────────────────────────
Write-Host ""
Write-Host "── Summary ─────────────────────────────────────────────────────────"
Write-Host "  Total tag refs   : $totalRefs"
Write-Host "  Resolved to SHA  : $resolved"
Write-Host "  Failed           : $unresolved"
Write-Host ""

if (-not $Apply) {
    Write-Host "Dry-run mode — no files modified.  Pass -Apply to rewrite workflow files." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Ready-to-use pinned references:"
    foreach ($r in $allResults | Where-Object { $_.SHA }) {
        Write-Host "  $($r.PinnedLine)"
    }
    exit 0
}

# ── Apply pinned SHAs ─────────────────────────────────────────────────────────
Write-Host "Applying pinned SHAs to workflow files..." -ForegroundColor Cyan

$byFile = $allResults | Where-Object { $_.SHA } | Group-Object File

foreach ($group in $byFile) {
    $filePath = Join-Path $WorkflowDir $group.Name
    $content  = Get-Content $filePath -Raw

    foreach ($r in $group.Group) {
        $old = $r.RawLine
        $new = "uses: $($r.Action)@$($r.SHA)  # $($r.TagRef)"
        $content = $content.Replace($old, $new)
    }

    if ($PSCmdlet.ShouldProcess($filePath, 'Write pinned SHAs')) {
        [System.IO.File]::WriteAllText($filePath, $content)
        Write-Host "  Updated: $($group.Name)" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "Done.  Commit the updated workflow files to lock in SHA pinning." -ForegroundColor Green
Write-Host "Dependabot will keep them current — see .github/dependabot.yml" -ForegroundColor Cyan
