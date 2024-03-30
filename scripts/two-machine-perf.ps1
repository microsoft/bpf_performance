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
    [string]$Duration = "60000"
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

$Job = Invoke-Command -Session $Session -ScriptBlock {
    param($RemoteDir, $Duration)
    $CtsTraffic = "$RemoteDir\cts-traffic\ctsTraffic.exe"
    &$CtsTraffic -listen:* -consoleverbosity:1 -timeLimit:$Duration
} -ArgumentList $RemoteDir, $Duration -AsJob

Write-Output "Starting the local ctsTraffic.exe for Send tests"
.\ctsTraffic.exe -target:$RemoteAddress -consoleverbosity:1 -statusfilename:SendStatus.csv -connectionfilename:SendConnections.csv -timeLimit:$Duration

Write-Output "Waiting for the remote job to complete"
Wait-Job $Job
Receive-Job $Job

Write-Output "Running the tests on the remote machine"
Write-Output "Starting the remote ctsTraffic.exe for Recv tests"

$Job = Invoke-Command -Session $Session -ScriptBlock {
    param($RemoteDir, $Duration)
    $CtsTraffic = "$RemoteDir\cts-traffic\ctsTraffic.exe"
    &$CtsTraffic -listen:* -consoleverbosity:1 -timeLimit:$Duration -pattern:pull
} -ArgumentList $RemoteDir, $Duration -AsJob

Write-Output "Starting the local ctsTraffic.exe for Recv tests"
.\ctsTraffic.exe -target:$RemoteAddress -consoleverbosity:1 -statusfilename:RecvStatus.csv -connectionfilename:RecvConnections.csv -pattern:pull -timeLimit:$Duration

Write-Output "Waiting for the remote job to complete"
Wait-Job $Job
Receive-Job $Job

$values = get-content .\SendConnections.csv | convertfrom-csv | select-object -Property SendBps | ForEach-Object { [long]($_.SendBps) }  | Sort-Object
$SendMedianBps = $values[$values.Length / 2]
Write-Output "Median SendBps: $SendMedianBps"

$values = get-content .\RecvConnections.csv | convertfrom-csv | select-object -Property RecvBps | ForEach-Object { [long]($_.RecvBps) }  | Sort-Object
$RecvMedianBps = $values[$values.Length / 2]
Write-Output "Median RecvBps: $RecvMedianBps"

Write-Output "Tests completed. Cleaning up..."

Remove-PSSession $Session

$CtsTrafficResults = @()

$CtsTrafficResults += [PSCustomObject]@{
    Timestamp = (Get-Date).ToUniversalTime().ToString("o")
    Test = "CtsTraffic Send"
    Metric = $SendMedianBps
}

$CtsTrafficResults += [PSCustomObject]@{
    Timestamp = (Get-Date).ToUniversalTime().ToString("o")
    Test = "CtsTraffic Recv"
    Metric = $RecvMedianBps
}

$CtsTrafficResults | Export-Csv -Path .\ctsTrafficResults.csv -NoTypeInformation
