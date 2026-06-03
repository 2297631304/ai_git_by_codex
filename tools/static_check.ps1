$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

Write-Host "== MSVC /analyze =="
& (Join-Path $PSScriptRoot "build.ps1") -Config Debug -Analyze | Out-Host

Write-Host "== XML static schema check =="
& (Join-Path $PSScriptRoot "xml_static_check.ps1")

Write-Host "== vector subscript candidates =="
$rg = Get-Command rg -ErrorAction SilentlyContinue
if ($rg) {
    $matches = & rg -n --glob "*.cpp" --glob "*.hpp" "\[[^\]]+\]" "$Root\src" "$Root\include" "$Root\tools"
    foreach ($match in $matches) {
        Write-Host "STATIC_BOUNDS_CANDIDATE $match"
    }
} else {
    Get-ChildItem "$Root\src","$Root\include","$Root\tools" -Recurse -Include *.cpp,*.hpp |
        Select-String -Pattern "\[[^\]]+\]" |
        ForEach-Object { Write-Host "STATIC_BOUNDS_CANDIDATE $($_.Path):$($_.LineNumber):$($_.Line.Trim())" }
}

$clangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue
if ($clangTidy) {
    Write-Host "== clang-tidy =="
    & clang-tidy "$Root\src\main.cpp" "$Root\src\config.cpp" "$Root\src\filter_core.cpp" -- -std=c++20 "-I$Root\include"
} else {
    Write-Host "clang-tidy not found; skipped"
}

$cppcheck = Get-Command cppcheck -ErrorAction SilentlyContinue
if ($cppcheck) {
    Write-Host "== cppcheck =="
    & cppcheck --enable=warning,style,performance,portability --std=c++20 "-I$Root\include" "$Root\src" "$Root\include"
} else {
    Write-Host "cppcheck not found; skipped"
}
