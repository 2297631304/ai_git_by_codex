$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

& (Join-Path $PSScriptRoot "xml_static_check.ps1")

$Exe = & (Join-Path $PSScriptRoot "build.ps1") -Config Release | Select-Object -Last 1
$Output = & $Exe (Join-Path $Root "config\esl_config.xml")
$Output | Write-Host

$SchemaPath = Join-Path $Root "include\esl\config_schema.hpp"
$SchemaText = Get-Content -Raw $SchemaPath
$Required = @([regex]::Matches($SchemaText, "X\(([A-Za-z_][A-Za-z0-9_]*)\s*,") | ForEach-Object {
    "field=$($_.Groups[1].Value)"
} | Select-Object -Unique)

foreach ($Field in $Required) {
    if (-not ($Output -match [regex]::Escape($Field))) {
        throw "Missing CONFIG_AUDIT for $Field"
    }
}

if (-not ($Output -match "VECTOR_AUDIT name=mask_bits .*storage=dynamic_bitset")) {
    throw "mask_bits was not converted to dynamic_bitset"
}

Write-Host "XML_AUDIT_OK"
