# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

param (
    [Parameter(Mandatory = $false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [Parameter(Mandatory = $false)]
    [ValidateSet("x64")]
    [string]$Arch = "x64",

    [Parameter(Mandatory = $false)]
    [string]$PeerName = "netperf-peer",

    [Parameter(Mandatory = $false)]
    [string]$RemotePSConfiguration = "PowerShell.7",

    [Parameter(Mandatory = $false)]
    [string]$RemoteDir = "C:\_work",

    [Parameter(Mandatory = $false)]
    [string]$Duration = "60000",

    [Parameter(Mandatory = $false)]
    [bool]$CpuProfile = $false
)

#Set-StrictMode -Version 'Latest'
$PSDefaultParameterValues['*:ErrorAction'] = 'Stop'
#$ProgressPreference = 'SilentlyContinue'

# Set up the connection to the peer over remote powershell.
Write-Output "Connecting to $PeerName..."
$Username = (Get-ItemProperty 'HKLM:\Software\Microsoft\Windows NT\CurrentVersion\Winlogon').DefaultUserName
$Password = (Get-ItemProperty 'HKLM:\Software\Microsoft\Windows NT\CurrentVersion\Winlogon').DefaultPassword | ConvertTo-SecureString -AsPlainText -Force
$Creds = New-Object System.Management.Automation.PSCredential ($Username, $Password)
$Session = New-PSSession -ComputerName $PeerName -Credential $Creds -ConfigurationName $RemotePSConfiguration
if ($null -eq $Session) {
    Write-Error "Failed to create remote session"
}

# Find all the local and remote IP and MAC addresses.
$RemoteAddress = [System.Net.Dns]::GetHostAddresses($Session.ComputerName)[0].IPAddressToString
Write-Output "Successfully connected to peer: $RemoteAddress"

Write-Output "Copying the tests to the remote machine"
Copy-Item -ToSession $Session . -Destination $RemoteDir\cts-traffic -Recurse -Force

Write-Output "Running the tests on the remote machine"
Write-Output "Starting the remote ctsTraffic.exe for Send tests"

# Set RSS on NIC on remote machine
$Job = Invoke-Command -Session $Session -ScriptBlock {
    $processor_count = $env:NUMBER_OF_PROCESSORS / 2

    $physical_nics = Get-NetAdapter | Where-Object -Property "PhysicalMediaType" -EQ "802.3"
    foreach ($nic in $physical_nics) {
        Set-NetAdapterRss -Name $nic.Name -MaxProcessors $processor_count -NumberOfReceiveQueues $processor_count
    }
}

# Set RSS on NIC on local machine
$physical_nics = Get-NetAdapter | Where-Object -Property "PhysicalMediaType" -EQ "802.3"
$processor_count = $env:NUMBER_OF_PROCESSORS / 2
foreach ($nic in $physical_nics) {
    Write-Output "Setting RSS on $nic.Name to $processor_count processors"
    Set-NetAdapterRss -Name $nic.Name -MaxProcessors $processor_count -NumberOfReceiveQueues $processor_count
}

$Job = Invoke-Command -Session $Session -ScriptBlock {
    param($RemoteDir, $Duration)
    $CtsTraffic = "$RemoteDir\cts-traffic\ctsTraffic.exe"
    &$CtsTraffic -listen:* -consoleverbosity:1 -timeLimit:$Duration -Buffer:1048576 -transfer:0xffffffffffff -MsgWaitAll:on  -Verify:connection -PrePostRecvs:3 -io:rioiocp
} -ArgumentList $RemoteDir, $Duration -AsJob

if ($CpuProfile) {
    wpr.exe -cancel
    Write-Output "Starting CPU profiling"
    wpr.exe -start CPU -filemode
}

Write-Output "Starting the local ctsTraffic.exe for Send tests"
.\ctsTraffic.exe -target:$RemoteAddress -consoleverbosity:1 -statusfilename:SendStatus.csv -connectionfilename:SendConnections.csv -timeLimit:$Duration -Buffer:1048576 -connections:32 -transfer:0xffffffffffff -MsgWaitAll:on  -Verify:connection -PrePostRecvs:3 -io:rioiocp

if ($CpuProfile) {
    Write-Output "Stopping CPU profiling"
    wpr.exe -stop cts_traffic_send.etl
}

Write-Output "Waiting for the remote job to complete"
Wait-Job $Job
Receive-Job $Job

Write-Output "Running the tests on the remote machine"
Write-Output "Starting the remote ctsTraffic.exe for Recv tests"

$Job = Invoke-Command -Session $Session -ScriptBlock {
    param($RemoteDir, $Duration)
    $CtsTraffic = "$RemoteDir\cts-traffic\ctsTraffic.exe"
    &$CtsTraffic -listen:* -consoleverbosity:1 -timeLimit:$Duration -pattern:pull -Buffer:1048576 -transfer:0xffffffffffff -MsgWaitAll:on  -Verify:connection -PrePostRecvs:3 -io:rioiocp
} -ArgumentList $RemoteDir, $Duration -AsJob

if ($CpuProfile) {
    Write-Output "Starting CPU profiling"
    wpr.exe -start CPU -filemode
}

Write-Output "Starting the local ctsTraffic.exe for Recv tests"
.\ctsTraffic.exe -target:$RemoteAddress -consoleverbosity:1 -statusfilename:RecvStatus.csv -connectionfilename:RecvConnections.csv -pattern:pull -timeLimit:$Duration -Buffer:1048576 -connections:32 -transfer:0xffffffffffff -MsgWaitAll:on  -Verify:connection -PrePostRecvs:3 -io:rioiocp

if ($CpuProfile) {
    Write-Output "Stopping CPU profiling"
    wpr.exe -stop cts_traffic_recv.etl
}

Write-Output "Waiting for the remote job to complete"
Wait-Job $Job
Receive-Job $Job

$values = get-content .\SendConnections.csv | convertfrom-csv | select-object -Property SendBps | ForEach-Object { [long]($_.SendBps) }  | Sort-Object -Descending
# If values is null or empty, set the peak to 0
if ($null -eq $values -or $values.Length -eq 0) {
    Write-Warning "No RecvBps values found"
    $SendPeakConnectionBps = 0
} else {
    $SendPeakConnectionBps = $values[0]
}
Write-Output "Peak SendConnectionBps: $SendPeakConnectionBps"

$values = get-content .\SendStatus.csv | convertfrom-csv | select-object -Property SendBps | ForEach-Object { [long]($_.SendBps) }  | Sort-Object -Descending
# If values is null or empty, set the median to 0
if ($null -eq $values -or $values.Length -eq 0) {
    Write-Warning "No RecvBps values found"
    $SendPeakBps = 0
} else {
    $SendPeakBps = $values[0]
}
Write-Output "Peak SendBps: $SendPeakBps"

$values = get-content .\RecvConnections.csv | convertfrom-csv | select-object -Property RecvBps | ForEach-Object { [long]($_.RecvBps) }  | Sort-Object -Descending
# If values is null or empty, set the median to 0
if ($null -eq $values -or $values.Length -eq 0) {
    Write-Warning "No RecvBps values found"
    $RecvPeakConnectionBps = 0
} else {
    $RecvPeakConnectionBps = $values[0]
}
Write-Output "Peak RecvConnectionBps: $RecvPeakConnectionBps"

$values = get-content .\RecvStatus.csv | convertfrom-csv | select-object -Property RecvBps | ForEach-Object { [long]($_.RecvBps) }  | Sort-Object -Descending
# If values is null or empty, set the median to 0
if ($null -eq $values -or $values.Length -eq 0) {
    Write-Warning "No RecvBps values found"
    $RecvPeakBps = 0
} else {
    $RecvPeakBps = $values[0]
}
$RecvPeakBps = $values[$values.Length / 2]
Write-Output "Peak RecvBps: $RecvPeakBps"

Write-Output "Tests completed. Cleaning up..."

# Set RSS on NIC on remote machine
$Job = Invoke-Command -Session $Session -ScriptBlock {
    $physical_nics = Get-NetAdapter | Where-Object -Property "PhysicalMediaType" -EQ "802.3"
    foreach ($nic in $physical_nics) {
        Set-NetAdapterRss -Name $nic.Name -MaxProcessors 8 -NumberOfReceiveQueues 8
    }
}

# Set RSS on NIC on local machine
$physical_nics = Get-NetAdapter | Where-Object -Property "PhysicalMediaType" -EQ "802.3"
foreach ($nic in $physical_nics) {
    Write-Output "Setting RSS on $nic.Name to 8 processors"
    Set-NetAdapterRss -Name $nic.Name -MaxProcessors 8 -NumberOfReceiveQueues 8
}


Write-Output "Debug logging"

Remove-PSSession $Session

$CtsTrafficResults = @()

$CtsTrafficResults += [PSCustomObject]@{
    Timestamp = (Get-Date).ToUniversalTime().ToString("o")
    Test = "CtsTraffic Send"
    Metric = $SendPeakBps
}

$CtsTrafficResults += [PSCustomObject]@{
    Timestamp = (Get-Date).ToUniversalTime().ToString("o")
    Test = "CtsTraffic Send Connection"
    Metric = $SendPeakConnectionBps
}

$CtsTrafficResults += [PSCustomObject]@{
    Timestamp = (Get-Date).ToUniversalTime().ToString("o")
    Test = "CtsTraffic Recv"
    Metric = $RecvPeakBps
}

$CtsTrafficResults += [PSCustomObject]@{
    Timestamp = (Get-Date).ToUniversalTime().ToString("o")
    Test = "CtsTraffic Recv Connection"
    Metric = $RecvPeakConnectionBps
}

$CtsTrafficResults | Export-Csv -Path .\ctsTrafficResults.csv -NoTypeInformation

exit 0