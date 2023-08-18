# Copyright (c) Microsoft Corporation
# SPDX-License-Identifier: MIT

# Script to pull artifact from latest scheduled run of the eBPF-For-Windows pipeline

param(
    [Parameter(Mandatory=$true)] [string] $ArtifactName,
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

Write-Output "Using run ID $runid in branch $Branch in repo $Owner/$Repo to fetch artifact $ArtifactName to $OutputPath."

gh run download $runid -R "$Owner/$Repo" -n $ArtifactName -D $OutputPath