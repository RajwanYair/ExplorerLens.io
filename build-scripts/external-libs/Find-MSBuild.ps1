$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootScript = Join-Path (Split-Path -Parent $scriptDir) 'Find-MSBuild.ps1'

if (-not (Test-Path $rootScript)) {
    throw "Find-MSBuild.ps1 not found at expected path: $rootScript"
}

& $rootScript
