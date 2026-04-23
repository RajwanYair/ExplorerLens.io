$ErrorActionPreference = 'SilentlyContinue'
Write-Host "`n=== REPO SIZE ===" -ForegroundColor Cyan
$tracked = git ls-files
Write-Host "Tracked files: $($tracked.Count)"

Write-Host "`n=== LARGE TRACKED FILES (>100KB, excl external/packages/build/x64) ===" -ForegroundColor Cyan
$tracked | Where-Object { $_ -notmatch '^(external|packages|build|x64)/' } | ForEach-Object {
    $fi = Get-Item -LiteralPath $_ -EA 0
    if ($fi -and $fi.Length -gt 100KB) { [pscustomobject]@{ KB=[math]::Round($fi.Length/1KB,1); Path=$_ } }
} | Sort-Object KB -Descending | Select-Object -First 25 | Format-Table -AutoSize

Write-Host "`n=== HEADER vs SOURCE (Engine/) ===" -ForegroundColor Cyan
$h = ($tracked | Where-Object { $_ -match '^Engine/.*\.h$' }).Count
$c = ($tracked | Where-Object { $_ -match '^Engine/.*\.cpp$' }).Count
Write-Host "Engine/ headers: $h"
Write-Host "Engine/ sources: $c"
Write-Host ("Ratio: {0:N2}:1" -f ($h/[Math]::Max($c,1)))

Write-Host "`n=== ENGINE TESTS ===" -ForegroundColor Cyan
$testFiles = Get-ChildItem Engine/Tests -Filter *.cpp -EA 0 | Sort-Object Length -Descending
Write-Host "Test .cpp files: $($testFiles.Count)"
foreach ($t in $testFiles) { "{0,7:N0} KB  {1,6} lines  {2}" -f ($t.Length/1KB), ((Get-Content $t.FullName).Count), $t.Name }
$runCount = (Select-String -Path "Engine/Tests/*.cpp" -Pattern '^\s*RUN_TEST\s*\(' -AllMatches | Measure-Object).Count
$testDefCount = (Select-String -Path "Engine/Tests/*.cpp" -Pattern '^\s*TEST\s*\(' -AllMatches | Measure-Object).Count
$assertCount = (Select-String -Path "Engine/Tests/*.cpp" -Pattern '\bASSERT\s*\(' -AllMatches | Measure-Object).Count
Write-Host "Total RUN_TEST() calls:  $runCount"
Write-Host "Total TEST() bodies:     $testDefCount"
Write-Host "Total ASSERT() uses:     $assertCount"

Write-Host "`n=== WORKFLOWS ===" -ForegroundColor Cyan
$wf = Get-ChildItem .github/workflows -Filter *.yml -EA 0 | Sort-Object Name
Write-Host "Workflow files: $($wf.Count)"
$wf | Select-Object @{n='KB';e={[math]::Round($_.Length/1KB,1)}}, Name | Format-Table -AutoSize

Write-Host "`n=== DOCS ===" -ForegroundColor Cyan
$md = $tracked | Where-Object { $_ -match '\.md$' -and $_ -notmatch '^external/' }
Write-Host "Markdown files (tracked, non-external): $($md.Count)"
Write-Host "Top folders:"
$md | ForEach-Object { ($_ -split '/')[0] } | Group-Object | Sort-Object Count -Descending | Select-Object Name, Count | Format-Table -AutoSize

Write-Host "`n=== DEAD CODE / CONSOLIDATION CANDIDATES ===" -ForegroundColor Cyan
@('src/LensServer','src/PluginHost','src/Tools.PSModule','Engine/Tests/FuzzTargets','Engine/Tests/gtest','Engine/AI','Engine/Enterprise','Engine/Media','Engine/Memory','Engine/Pipeline','Engine/Plugin','Engine/PluginHost','Engine/CLI','Engine/Codec','packaging/inno','packaging/nsis','packaging/msix','packaging/vdproj','LensServer','src','Engine/Tests/fuzz') | ForEach-Object {
    $exists = Test-Path $_
    $files = if ($exists) { (Get-ChildItem $_ -Recurse -File -EA 0).Count } else { 0 }
    "{0,6} files  {1,-45}  {2}" -f $files, $_, $(if($exists){'EXISTS'}else{'gone'})
}

Write-Host "`n=== index.html ===" -ForegroundColor Cyan
@('index.html','docs/index.html') | ForEach-Object {
    if (Test-Path $_) { "EXISTS  $_ ($([math]::Round((Get-Item $_).Length/1KB,1)) KB)" } else { "MISSING $_" }
}

Write-Host "`n=== GITHUB AI ASSETS ===" -ForegroundColor Cyan
"instructions: " + (Get-ChildItem .github/instructions -Filter *.md -EA 0).Count
"prompts:      " + (Get-ChildItem .github/prompts -Filter *.md -EA 0).Count
"agents:       " + (Get-ChildItem .github/agents -Filter *.md -EA 0).Count
"skills dirs:  " + (Get-ChildItem .github/skills -Directory -EA 0).Count
"issue templ:  " + (Get-ChildItem .github/ISSUE_TEMPLATE -EA 0).Count

Write-Host "`n=== VERSION + GIT ===" -ForegroundColor Cyan
"VERSION: " + (Get-Content VERSION -Raw).Trim()
"Branch:  " + (git rev-parse --abbrev-ref HEAD)
Write-Host "Uncommitted (first 20):"
git status --short | Select-Object -First 20

Write-Host "`n=== ACTION VERSIONS IN WORKFLOWS ===" -ForegroundColor Cyan
Select-String -Path ".github/workflows/*.yml" -Pattern 'uses:\s+\S+@v?\d' |
    ForEach-Object { ($_.Line -replace '^\s*uses:\s+','').Trim() } |
    Sort-Object -Unique | Select-Object -First 40

Write-Host "`n=== DONE ===" -ForegroundColor Green
