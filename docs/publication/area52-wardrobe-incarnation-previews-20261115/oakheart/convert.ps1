$ErrorActionPreference='Stop'
$d=$PSScriptRoot
$out=Join-Path $d 'candidate/creature/vsancient'
$null=New-Item -ItemType Directory -Force "$out/tree47640"
$files=(Get-Content "$d/manifest.json" -Raw | ConvertFrom-Json).files
$names=Get-Content "$d/names.json" -Raw | ConvertFrom-Json -AsHashtable
$cases=@()
foreach($file in $files){
    $src=Join-Path $d $file.file
    if ((Get-FileHash $src).Hash -ne $file.sha256){throw "Input hash mismatch"}
    $name=Split-Path $names[[string]$file.fileDataID] -Leaf
    $virtual="creature/vsancient/$name"
    if($name.EndsWith('.blp')){$virtual="creature/vsancient/tree47640/$name"}
    Copy-Item -LiteralPath $src -Destination (Join-Path "$d/candidate" $virtual) -Force
    $cases+='case '+$file.fileDataID+': return "'+$virtual+'";'
}
$adapter='using System; namespace MultiConverter.Lib.Converters { public static class Listfile { public static string LookupFilename(uint id,string suffix){ switch(id){'+($cases -join '')+'default:throw new ArgumentException();}}} public static class ChunkedWowFile{ public static int InvertedMagicToInt(string s){return BitConverter.ToInt32(System.Text.Encoding.ASCII.GetBytes(s),0);}}}'
$src=Get-Content "$d/ModelConverter.cs" -Raw
# The upstream texture writer otherwise starts at the pre-skin length and overwrites appended lookups.
$src=$src.Replace('FixTXID();','{ dataSize = Data.Length; FixTXID(); }').Replace('FilenameLength = filename.Length','FilenameLength = filename.Length + 1')
Set-Content "$d/ModelConverter.offline.cs" $src
Set-Content "$d/OfflinePaths.cs" $adapter
Add-Type -Path "$d/OfflinePaths.cs","$d/ModelConverter.offline.cs","$d/SkinConverter.cs","$d/WowFile.cs" -IgnoreWarnings
$cwd=[Environment]::CurrentDirectory
try{
    [Environment]::CurrentDirectory=$out
    $model=[MultiConverter.Lib.Converters.M2Converter]::new("$out/vsnightmareancient.m2",$false)
    if(-not $model.Fix()){throw "Conversion failed"}
    # The retail name starts where the WotLK blend-override descriptor is written.
    $original=[IO.File]::ReadAllBytes("$d/vsnightmareancient.m2")
    $len=[BitConverter]::ToInt32($original,16)
    $offset=[BitConverter]::ToInt32($original,20)+8
    $end=$model.Data.Length
    $model.AddEmptyBytes($end,$len)
    [Array]::Copy($original,$offset,$model.Data,$end,$len)
    [Array]::Copy([BitConverter]::GetBytes([int]$end),0,$model.Data,12,4)
    $model.Save()
}finally{[Environment]::CurrentDirectory=$cwd}


& 'C:/Users/danie/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' "$d/finish_effects.py"
if($LASTEXITCODE){throw "Nightmare Ancient effects conversion failed"}
