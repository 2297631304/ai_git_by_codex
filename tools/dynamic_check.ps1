param(
    [int]$TimeoutSeconds = 5
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"

function Invoke-Case {
    param(
        [string]$Name,
        [string]$Exe,
        [string]$ConfigPath,
        [ValidateSet("success", "bounds_error", "timeout", "bounds_or_timeout")]
        [string]$Expect,
        [int]$TimeoutSeconds
    )

    $stdout = Join-Path $BuildDir "$Name.stdout.log"
    $stderr = Join-Path $BuildDir "$Name.stderr.log"
    Remove-Item -LiteralPath $stdout, $stderr -Force -ErrorAction SilentlyContinue

    Write-Host "DYNAMIC_CASE_START name=$Name config=$ConfigPath expect=$Expect timeout_sec=$TimeoutSeconds"
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $Exe
    $psi.Arguments = "`"$ConfigPath`""
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $psi
    [void]$process.Start()
    $finished = $process.WaitForExit($TimeoutSeconds * 1000)

    if (-not $finished) {
        try {
            $process.Kill()
        } catch {
            Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
        }
        Start-Sleep -Milliseconds 200
        $out = $process.StandardOutput.ReadToEnd()
        $err = $process.StandardError.ReadToEnd()
        Set-Content -Path $stdout -Value $out -Encoding UTF8
        Set-Content -Path $stderr -Value $err -Encoding UTF8
        if ($out) { $out | Write-Host }
        if ($err) { $err | Write-Host }
        Write-Host "TIMEOUT_DETECTED name=$Name config=$ConfigPath timeout_sec=$TimeoutSeconds"
        if ($Expect -ne "timeout" -and $Expect -ne "bounds_or_timeout") {
            throw "Case $Name timed out unexpectedly"
        }
        Write-Host "DYNAMIC_TIMEOUT_OK name=$Name"
        return
    }

    $out = $process.StandardOutput.ReadToEnd()
    $err = $process.StandardError.ReadToEnd()
    $exitCode = $process.ExitCode
    Set-Content -Path $stdout -Value $out -Encoding UTF8
    Set-Content -Path $stderr -Value $err -Encoding UTF8
    if ($out) { $out | Write-Host }
    if ($err) { $err | Write-Host }

    $combined = "$out`n$err"
    $hasBounds = $combined -match "BOUNDS_ERROR|vector subscript out of range|AddressSanitizer|UndefinedBehaviorSanitizer|heap-buffer-overflow|stack-buffer-overflow|out_of_range"

    Write-Host "DYNAMIC_CASE_EXIT name=$Name exit=$exitCode bounds_signal=$hasBounds"

    if ($Expect -eq "success" -and $exitCode -ne 0) {
        throw "Case $Name expected success, got exit $exitCode"
    }
    if ($Expect -eq "bounds_error" -and -not $hasBounds) {
        throw "Case $Name expected bounds error, but no bounds signal was observed"
    }
    if ($Expect -eq "timeout") {
        throw "Case $Name expected timeout, but process exited"
    }
    if ($Expect -eq "bounds_or_timeout" -and -not $hasBounds) {
        throw "Case $Name expected debug iterator/sanitizer bounds signal or timeout, but no signal was observed"
    }

    if ($hasBounds) {
        Write-Host "DYNAMIC_BOUNDS_OK name=$Name"
    }
}

Write-Host "== Debug runtime build =="
$Exe = & (Join-Path $PSScriptRoot "build.ps1") -Config Debug | Select-Object -Last 1

Invoke-Case -Name "safe" -Exe $Exe -ConfigPath (Join-Path $Root "config\esl_config.xml") -Expect "success" -TimeoutSeconds $TimeoutSeconds
Invoke-Case -Name "checked_oob" -Exe $Exe -ConfigPath (Join-Path $Root "config\esl_config_oob_checked.xml") -Expect "bounds_error" -TimeoutSeconds $TimeoutSeconds
Invoke-Case -Name "unchecked_oob" -Exe $Exe -ConfigPath (Join-Path $Root "config\esl_config_unchecked_oob.xml") -Expect "bounds_or_timeout" -TimeoutSeconds $TimeoutSeconds
Invoke-Case -Name "hang" -Exe $Exe -ConfigPath (Join-Path $Root "config\esl_config_hang.xml") -Expect "timeout" -TimeoutSeconds $TimeoutSeconds

Write-Host "DYNAMIC_AUDIT_OK"
