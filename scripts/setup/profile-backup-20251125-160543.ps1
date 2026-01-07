# DarkThumbs Minimal Profile
[System.Net.Http.HttpClient]::DefaultProxy = New-Object System.Net.WebProxy('http://proxy-dmz.intel.com:912', $true)
Write-Host 'DarkThumbs environment loaded' -ForegroundColor Green
function dt { Set-Location 'C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs' }
