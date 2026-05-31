$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Exe = & (Join-Path $PSScriptRoot "build.ps1") -Config Release | Select-Object -Last 1
$Output = & $Exe (Join-Path $Root "config\esl_config.xml")
$Output | Write-Host

$Required = @(
    "field=model_name",
    "field=clock_period_ns",
    "field=cycles",
    "field=threshold",
    "field=gain",
    "field=enable_trace",
    "field=input_sequence",
    "field=mask_bits"
)

foreach ($Field in $Required) {
    if (-not ($Output -match [regex]::Escape($Field))) {
        throw "Missing CONFIG_AUDIT for $Field"
    }
}

if (-not ($Output -match "VECTOR_AUDIT name=mask_bits .*storage=dynamic_bitset")) {
    throw "mask_bits was not converted to dynamic_bitset"
}

Write-Host "XML_AUDIT_OK"
