param(
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",
    [switch]$Analyze
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$VcVars = "D:\Software\Visual Studio\anzhuang\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $VcVars)) {
    $VsWhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $VsWhere) {
        $VsPath = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($VsPath) {
            $Candidate = Join-Path $VsPath "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $Candidate) {
                $VcVars = $Candidate
            }
        }
    }
}
if (-not (Test-Path $VcVars)) {
    throw "Cannot find vcvars64.bat. Install Visual Studio C++ build tools or update tools/build.ps1."
}

$Exe = Join-Path $BuildDir "esl_demo.exe"
$Rsp = Join-Path $BuildDir "cl.rsp"
$Sources = @(
    (Join-Path $Root "src\main.cpp"),
    (Join-Path $Root "src\config.cpp"),
    (Join-Path $Root "src\filter_core.cpp")
)

$Flags = @(
    "/nologo",
    "/std:c++20",
    "/EHsc",
    "/W4",
    "/WX",
    "/utf-8",
    "/FS",
    "/Fd$BuildDir\esl_demo.pdb",
    "/I$Root\include",
    "/Fe$Exe",
    "/Fo$BuildDir\"
)

if ($Config -eq "Debug") {
    $Flags += @("/Od", "/Zi", "/RTC1", "/MDd", "/D_DEBUG", "/D_ITERATOR_DEBUG_LEVEL=2")
} else {
    $Flags += @("/O2", "/MD", "/DNDEBUG")
}
if ($Analyze) {
    $Flags += "/analyze"
}

$RspLines = $Flags + $Sources + @("/link", "/DEBUG")
Set-Content -Path $Rsp -Value $RspLines -Encoding ASCII

$Cmd = "`"$VcVars`" >nul && cl @`"$Rsp`""
cmd.exe /d /c $Cmd
if ($LASTEXITCODE -ne 0) {
    throw "MSVC build failed with exit code $LASTEXITCODE"
}

Write-Host "BUILD_OK $Exe"
Write-Output $Exe
