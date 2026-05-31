param(
    [string]$ConfigPath = ""
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Exe = & (Join-Path $PSScriptRoot "build.ps1") -Config Release | Select-Object -Last 1
if (-not $ConfigPath) {
    $ConfigPath = Join-Path $Root "config\esl_config.xml"
}

& $Exe $ConfigPath
if ($LASTEXITCODE -ne 0) {
    throw "Simulation failed with exit code $LASTEXITCODE"
}
