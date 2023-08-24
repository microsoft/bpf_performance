# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

# Script to pull artifact from latest scheduled run of the eBPF-For-Windows pipeline


param(
    # Set of artifacts to download. This is a list of strings, each of which is the name of an artifact to download.
    [Parameter(Mandatory=$true)] [string[]] $ArtifactsToDownload,
    [Parameter(Mandatory=$true)] [string] $OutputPath,
    [Parameter(Mandatory=$false)] [string] $Owner = "microsoft",
    [Parameter(Mandatory=$false)] [string] $Repo = "ebpf-for-windows",
    [Parameter(Mandatory=$false)] [string] $Branch = "main",
    [Parameter(Mandatory=$false)] [string] $RunId = $null)

if ($null -eq (Get-Command 'gh.exe' -ErrorAction SilentlyContinue)) {
    throw "Unable to locate gh.exe. This command to have the GitHub CLI installed and in your path."
}

if (!$runid) {
    $run = ((Invoke-WebRequest -Uri  "https://api.github.com/repos/$Owner/$Repo/actions/runs?per_page=1&exclude_pull_requests=true&branch=$Branch&status=completed&event=schedule").Content | ConvertFrom-Json)
    $runid = $run.workflow_runs[0].id
}

# if $OutputPath\runid.txt exists and contains the same runid, we've already downloaded this artifact
if (Test-Path "$OutputPath\runid.txt") {
    $oldrunid = Get-Content "$OutputPath\runid.txt"
    if ($oldrunid -eq $runid) {
        Write-Output "Artifact $ArtifactName from run $runid already downloaded to $OutputPath."
        return
    }
}

# Remove all files in the $OutputPath without prompting
Remove-Item -Path $OutputPath\* -Recurse -Force -ErrorAction SilentlyContinue

Write-Output "Using run ID $runid in branch $Branch in repo $Owner/$Repo to fetch artifact $ArtifactsToDownload to $OutputPath."

# Expand ArtifactsToDownload into a string and add -n to the beginning of each one

$ArtifactsToDownload | ForEach-Object {
    gh run download $runid -R "$Owner/$Repo" -n $_ -D $OutputPath
    # Throw if exit code != 0
    if ($LASTEXITCODE -ne 0) {
        throw "gh run download failed with exit code $LASTEXITCODE"
    }
}

# Write the runid to a file so we can check it next time
Set-Content -Path "$OutputPath\runid.txt" -Value $runid
