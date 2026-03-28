Set-Location "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io"
$log = "build-logs\fix13-run.log"
Write-Host "Starting build, log: $log"
& "build-scripts\build-and-log.bat" $log
$code = $LASTEXITCODE
Write-Host "Exit code: $code"
