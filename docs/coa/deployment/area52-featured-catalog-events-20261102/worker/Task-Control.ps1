param(
    [Parameter(Mandatory=$true)][string]$Before,
    [Parameter(Mandatory=$true)][string]$After,
    [ValidateSet('inspect','preflight','pause','install','verify','enable','rollback-check','rollback')][string]$Phase='inspect'
)
$ErrorActionPreference='Stop'
$name='Area52 Featured Catalog'
function Normalize([string]$text) {
    $service=New-Object -ComObject 'Schedule.Service'
    $service.Connect()
    $definition=$service.NewTask(0)
    $definition.XmlText=$text
    $definition.Settings.Enabled=$true
    [xml]$xml=$definition.XmlText
    foreach ($node in $xml.SelectNodes('//*[local-name()="UserId"]')) {
        if (-not $node.InnerText.StartsWith('S-1-')) { $node.InnerText=([Security.Principal.NTAccount]::new($node.InnerText)).Translate([Security.Principal.SecurityIdentifier]).Value }
    }
    return $xml.Task.OuterXml
}
$old=[IO.File]::ReadAllText($Before)
$new=[IO.File]::ReadAllText($After)
$task=Get-ScheduledTask -TaskName $name -ErrorAction Stop
$current=Normalize (Export-ScheduledTask -TaskName $name)
$oldMatches=$current -eq (Normalize $old)
$newMatches=$current -eq (Normalize $new)
if ($Phase -ne 'inspect') {
    $principal=New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Run the activation command from Administrator PowerShell; task migration requires it.' }
}
switch ($Phase) {
    {$_ -in 'inspect','preflight'} {
        if (-not $oldMatches -or -not $task.Settings.Enabled) { throw 'Catalog task baseline changed.' }
        if ($Phase -eq 'preflight' -and [string]$task.State -eq 'Running') { throw 'Catalog worker is busy; let its current run finish before activation.' }
    }
    'pause' {
        if (-not $oldMatches) { throw 'Catalog task changed before pause.' }
        Disable-ScheduledTask -TaskName $name | Out-Null
        $deadline=(Get-Date).AddSeconds(120)
        while ([string](Get-ScheduledTask -TaskName $name).State -eq 'Running') {
            if ((Get-Date) -ge $deadline) { throw 'Catalog run is still busy; it was not terminated.' }
            Start-Sleep -Seconds 1
        }
    }
    'install' {
        if (-not $oldMatches -or $task.Settings.Enabled -or [string]$task.State -eq 'Running') { throw 'Catalog task must be paused before migration.' }
        Register-ScheduledTask -TaskName $name -Xml $new -Force | Out-Null
    }
    'verify' {
        if (-not $newMatches -or $task.Settings.Enabled) { throw 'Disabled event task readback failed.' }
    }
    'enable' {
        if (-not $newMatches) { throw 'Event task changed before enabling.' }
        Enable-ScheduledTask -TaskName $name | Out-Null
        if ([string](Get-ScheduledTask -TaskName $name).State -ne 'Running') { Start-ScheduledTask -TaskName $name }
    }
    {$_ -in 'rollback-check','rollback'} {
        if ((-not $newMatches -and -not $oldMatches) -or [string]$task.State -eq 'Running') { throw 'Preserve changed or busy task for recovery.' }
        if ($Phase -eq 'rollback') { Register-ScheduledTask -TaskName $name -Xml $old -Force | Out-Null }
    }
}
$final=Get-ScheduledTask -TaskName $name
$expected=if ($Phase -eq 'rollback-check') { $current } elseif ($Phase -in 'install','verify','enable') { Normalize $new } else { Normalize $old }
if ((Normalize (Export-ScheduledTask -TaskName $name)) -ne $expected) { throw 'Task definition readback mismatch.' }
if ($Phase -in 'pause','install','verify' -and $final.Settings.Enabled) { throw 'Task unexpectedly enabled.' }
if ($Phase -in 'enable','rollback' -and -not $final.Settings.Enabled) { throw 'Task unexpectedly disabled.' }
Write-Output ('A52_EVENT_TASK_OK:'+$Phase)
