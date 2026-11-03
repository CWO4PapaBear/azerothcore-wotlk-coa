param([Parameter(Mandatory=$true)][string]$Before,[ValidateSet('preflight','disable','verify','enable')][string]$Phase='preflight')
$ErrorActionPreference='Stop'
$name='Area52 Featured Catalog'
$principal=New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Administrator PowerShell is required for catalog maintenance.' }
function Normalize([string]$text) {
    $service=New-Object -ComObject 'Schedule.Service';$service.Connect()
    $definition=$service.NewTask(0);$definition.XmlText=$text;$definition.Settings.Enabled=$true
    [xml]$xml=$definition.XmlText
    foreach ($node in $xml.SelectNodes('//*[local-name()="UserId"]')) {
        if (-not $node.InnerText.StartsWith('S-1-')) { $node.InnerText=([Security.Principal.NTAccount]::new($node.InnerText)).Translate([Security.Principal.SecurityIdentifier]).Value }
    }
    return $xml.Task.OuterXml
}
if ((Normalize (Export-ScheduledTask -TaskName $name)) -ne (Normalize ([IO.File]::ReadAllText($Before)))) { throw 'Catalog task baseline changed.' }
switch ($Phase) {
    'preflight' { if (-not (Get-ScheduledTask -TaskName $name).Settings.Enabled) { throw 'Catalog task unexpectedly disabled.' } }
    'disable' { Disable-ScheduledTask -TaskName $name | Out-Null }
    'verify' { if ((Get-ScheduledTask -TaskName $name).Settings.Enabled) { throw 'Catalog task unexpectedly enabled.' } }
    'enable' { Enable-ScheduledTask -TaskName $name | Out-Null; if ([string](Get-ScheduledTask -TaskName $name).State -ne 'Running') { Start-ScheduledTask -TaskName $name } }
}
$task=Get-ScheduledTask -TaskName $name
if ($Phase -in 'disable','verify' -and $task.Settings.Enabled) { throw 'Disabled catalog task readback failed.' }
Write-Output ('A52_EDITOR_TASK_OK:'+$Phase)
