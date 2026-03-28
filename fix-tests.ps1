$root = 'c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io'
$path = "$root\Engine\Tests\EngineTests.cpp"
$content = [System.IO.File]::ReadAllText($path, [System.Text.UTF8Encoding]::new($false))

$before1 = ([regex]::Matches($content, 'ExplorerLens::Engine::Decoders::ArchiveFormat')).Count
$content = $content -replace 'ExplorerLens::Engine::Decoders::ArchiveFormat', 'ExplorerLens::Engine::Decoders::GridArchiveFormat'
$after1 = ([regex]::Matches($content, 'ExplorerLens::Engine::Decoders::GridArchiveFormat')).Count
Write-Host "Fix1 ArchiveFormat->GridArchiveFormat: $before1->$after1"

$before2 = ([regex]::Matches($content, 'ExplorerLens::Engine::Decoders::ColorSpace')).Count
$content = $content -replace 'ExplorerLens::Engine::Decoders::ColorSpace', 'ExplorerLens::Engine::Decoders::ManagedColorSpace'
$after2 = ([regex]::Matches($content, 'ExplorerLens::Engine::Decoders::ManagedColorSpace')).Count
Write-Host "Fix2 ColorSpace->ManagedColorSpace: $before2->$after2"

$before3 = ([regex]::Matches($content, '(?<!Decoders::)EBookFormat::')).Count
$content = [regex]::Replace($content, '(?<!Decoders::)EBookFormat::', 'DecoderEBookFormat::')
$after3 = ([regex]::Matches($content, 'DecoderEBookFormat::')).Count
$remaining = ([regex]::Matches($content, [regex]::Escape('EBookFormat::'))).Count
Write-Host "Fix3 EBookFormat->DecoderEBookFormat: $before3->$after3 (remaining qualified: $remaining)"

[System.IO.File]::WriteAllText($path, $content, [System.Text.UTF8Encoding]::new($false))
Write-Host "DONE"
