# Set-RepoTopics.ps1 — Set GitHub repository topics for ExplorerLens.io
# Copyright (c) 2026 ExplorerLens Project
#
# Requires: GitHub CLI (gh) authenticated, or a GITHUB_TOKEN env var.
# Usage:
#   .\scripts\Set-RepoTopics.ps1
#   .\scripts\Set-RepoTopics.ps1 -DryRun
#   .\scripts\Set-RepoTopics.ps1 -Owner myorg -Repo myrepo
#
[CmdletBinding()]
param(
    [string] $Owner       = '',
    [string] $Repo        = '',
    [switch] $DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Repository topics — aligned with GitHub's discoverability taxonomy
# ---------------------------------------------------------------------------
$Topics = @(
    'windows-shell-extension',
    'thumbnail-provider',
    'cpp20',
    'directx',
    'gpu-accelerated',
    'com-dll',
    'windows-explorer',
    'ithumbnailprovider',
    'image-viewer',
    'file-preview'
)

# ---------------------------------------------------------------------------
# Resolve owner/repo from git remote if not provided
# ---------------------------------------------------------------------------
if (-not $Owner -or -not $Repo) {
    $remote = git remote get-url origin 2>$null
    if ($remote -match 'github\.com[:/](?<owner>[^/]+)/(?<repo>[^/\.]+)(\.git)?$') {
        if (-not $Owner) { $Owner = $Matches['owner'] }
        if (-not $Repo)  { $Repo  = $Matches['repo']  }
    }
}

if (-not $Owner -or -not $Repo) {
    Write-Error "Cannot determine GitHub owner/repo. Pass -Owner and -Repo explicitly, or ensure 'origin' remote points to github.com."
}

Write-Host ""
Write-Host "Repository  : $Owner/$Repo" -ForegroundColor Cyan
Write-Host "Topics      : $($Topics -join ', ')" -ForegroundColor Cyan
Write-Host ""

if ($DryRun) {
    Write-Host "[DryRun] Would PUT the above topics — no changes made." -ForegroundColor Yellow
    exit 0
}

# ---------------------------------------------------------------------------
# Prefer GitHub CLI (gh) — avoids PAT management
# ---------------------------------------------------------------------------
$ghCmd = Get-Command gh -ErrorAction SilentlyContinue
if ($ghCmd) {
    Write-Host "Using GitHub CLI (gh)..." -ForegroundColor Green
    $topicArgs = $Topics | ForEach-Object { "--add-topic=$_" }
    # gh repo edit accepts --add-topic but replaces require separate call; use API directly
    $topicsJson = ConvertTo-Json @{ names = $Topics } -Compress
    $result = $topicsJson | gh api "repos/$Owner/$Repo/topics" `
        --method PUT `
        --input - `
        --header "Accept: application/vnd.github+json" 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "gh api call failed: $result"
    }
    Write-Host "Topics updated via GitHub CLI." -ForegroundColor Green
    exit 0
}

# ---------------------------------------------------------------------------
# Fallback: GitHub REST API via GITHUB_TOKEN env var
# ---------------------------------------------------------------------------
$token = $env:GITHUB_TOKEN
if (-not $token) {
    Write-Error "GitHub CLI (gh) not found and GITHUB_TOKEN is not set. Install gh or set GITHUB_TOKEN."
}

Write-Host "Using GITHUB_TOKEN for REST API call..." -ForegroundColor Green

$uri     = "https://api.github.com/repos/$Owner/$Repo/topics"
$headers = @{
    Authorization = "Bearer $token"
    Accept        = "application/vnd.github+json"
    'X-GitHub-Api-Version' = '2022-11-28'
}
$body = ConvertTo-Json @{ names = $Topics } -Compress

$response = Invoke-RestMethod -Method PUT -Uri $uri -Headers $headers -Body $body -ContentType 'application/json'
Write-Host "Topics updated via REST API:" -ForegroundColor Green
$response.names | ForEach-Object { Write-Host "  - $_" -ForegroundColor Gray }
