param([Parameter(Mandatory=$true)][string]$Runtime)
$ErrorActionPreference='Stop'
$linuxRoot=& wsl.exe -d dml-arch -u root -- wslpath -a $Runtime.Replace('\','/')
if ($LASTEXITCODE -ne 0) { throw 'Unable to resolve catalog runtime.' }
& wsl.exe -d dml-arch -u root --exec python3 ($linuxRoot.Trim()+'/catalog_events.py') --config ($linuxRoot.Trim()+'/config.json') --signal '/home/dml/games/ascension-freepick-test/private/runtime-logs/area52-featured-catalog.signal'
if ($LASTEXITCODE -ne 0) { throw 'Catalog event listener stopped; retained job and receipts require inspection.' }
