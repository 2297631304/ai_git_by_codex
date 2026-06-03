$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$SchemaPath = Join-Path $Root "include\esl\config_schema.hpp"

if (-not (Test-Path $SchemaPath)) {
    throw "Missing C++ config schema: $SchemaPath"
}

$schemaText = Get-Content -Raw $SchemaPath
$fieldMatches = [regex]::Matches($schemaText, "X\(([A-Za-z_][A-Za-z0-9_]*)\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\)")
$schema = [ordered]@{}
foreach ($match in $fieldMatches) {
    $name = $match.Groups[1].Value
    $type = $match.Groups[2].Value
    if (-not $schema.Contains($name)) {
        $schema[$name] = $type
    }
}
if ($schema.Count -eq 0) {
    throw "No fields found in C++ config schema"
}

$xmlFiles = @(Get-ChildItem -Path (Join-Path $Root "config") -Filter "*.xml" -File)
if ($xmlFiles.Count -eq 0) {
    throw "No XML config files found under config/"
}

$hasError = $false
foreach ($file in $xmlFiles) {
    $xmlText = Get-Content -Raw $file.FullName
    $elementMatches = [regex]::Matches($xmlText, "<([A-Za-z_][A-Za-z0-9_]*)>[^<]*</\1>")
    $xmlFields = @()
    foreach ($match in $elementMatches) {
        $name = $match.Groups[1].Value
        if ($name -ne "esl_model") {
            $xmlFields += $name
        }
    }

    $groups = $xmlFields | Group-Object
    foreach ($group in $groups) {
        if ($group.Count -gt 1) {
            Write-Host "XML_SCHEMA_MISMATCH config=$($file.Name) duplicate_field=$($group.Name)"
            $hasError = $true
        }
    }

    foreach ($field in $schema.Keys) {
        if ($xmlFields -notcontains $field) {
            Write-Host "XML_SCHEMA_MISMATCH config=$($file.Name) missing_cpp_field=$field"
            $hasError = $true
        } else {
            Write-Host "XML_STATIC_FIELD config=$($file.Name) field=$field type=$($schema[$field]) status=matched"
        }
    }

    foreach ($field in $xmlFields) {
        if (-not $schema.Contains($field)) {
            Write-Host "XML_SCHEMA_MISMATCH config=$($file.Name) extra_xml_field=$field"
            $hasError = $true
        }
    }
}

if ($hasError) {
    throw "XML static schema check failed"
}

Write-Host "XML_STATIC_OK files=$($xmlFiles.Count) fields=$($schema.Count)"
