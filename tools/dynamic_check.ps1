param(
    [switch]$RunBoundsProbe
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"

Write-Host "== Debug runtime build =="
$Exe = & (Join-Path $PSScriptRoot "build.ps1") -Config Debug | Select-Object -Last 1

Write-Host "== Run ESL simulation under debug runtime settings =="
& $Exe (Join-Path $Root "config\esl_config.xml")
if ($LASTEXITCODE -ne 0) {
    throw "Simulation failed with exit code $LASTEXITCODE"
}

if ($RunBoundsProbe) {
    Write-Host "== Intentional bounds probe =="
    $ProbeExe = Join-Path $BuildDir "bounds_probe.exe"
    $Rsp = Join-Path $BuildDir "bounds_probe.rsp"
    $VcVars = "D:\Software\Visual Studio\anzhuang\VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $VcVars)) {
        throw "Cannot find vcvars64.bat"
    }
    $RspLines = @(
        "/nologo",
        "/std:c++20",
        "/EHsc",
        "/W4",
        "/WX",
        "/utf-8",
        "/Od",
        "/Zi",
        "/RTC1",
        "/MDd",
        "/D_DEBUG",
        "/D_ITERATOR_DEBUG_LEVEL=2",
        "/I$Root\include",
        "/Fe$ProbeExe",
        "/Fo$BuildDir\",
        (Join-Path $Root "tools\bounds_probe.cpp"),
        "/link",
        "/DEBUG"
    )
    Set-Content -Path $Rsp -Value $RspLines -Encoding ASCII
    $Cmd = "`"$VcVars`" >nul && cl @`"$Rsp`""
    cmd.exe /d /c $Cmd
    if ($LASTEXITCODE -ne 0) {
        throw "Bounds probe build failed with exit code $LASTEXITCODE"
    }
    & $ProbeExe
    if ($LASTEXITCODE -ne 0) {
        throw "Bounds probe did not report the expected detected error"
    }
}
