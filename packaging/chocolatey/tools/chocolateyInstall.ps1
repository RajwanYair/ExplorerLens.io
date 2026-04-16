$ErrorActionPreference = 'Stop'

$packageArgs = @{
  packageName    = 'explorerlens'
  fileType       = 'msi'
  url64bit       = 'https://github.com/RajwanYair/ExplorerLens/releases/download/v35.5.0/ExplorerLens-35.5.0-x64.msi'
  checksum64     = 'PLACEHOLDER_SHA256_REPLACE_ON_RELEASE'
  checksumType64 = 'sha256'
  silentArgs     = '/quiet /norestart'
  validExitCodes = @(0, 3010)
}

Install-ChocolateyPackage @packageArgs

Write-Host "ExplorerLens installed. Open Windows Explorer to verify thumbnails." -ForegroundColor Green
