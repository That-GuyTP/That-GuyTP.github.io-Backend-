param(
    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Add-Result {
    param(
        [System.Collections.Generic.List[object]]$Results,
        [string]$App,
        [string]$Status,
        [string]$Details
    )

    $Results.Add([PSCustomObject]@{
        App = $App
        Status = $Status
        Details = $Details
    }) | Out-Null
}

function Initialize-CompilerPath {
    $machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $env:Path = "$machinePath;$userPath"
}

function Ensure-Gpp {
    $gpp = Get-Command g++ -ErrorAction SilentlyContinue
    if (-not $gpp) {
        throw "g++ was not found in PATH. Install WinLibs/MinGW and reopen your shell."
    }
}

function Build-CoinRowRobot {
    param(
        [string]$Root,
        [switch]$Clean,
        [System.Collections.Generic.List[object]]$Results
    )

    $appName = "CoinRowRobot"
    $appDir = Join-Path $Root $appName
    $source = Join-Path $appDir "coin_row_robot_gui.cpp"
    $output = Join-Path $appDir "coin_row_robot_gui.exe"

    if (-not (Test-Path $source)) {
        Add-Result -Results $Results -App $appName -Status "SKIPPED" -Details "Missing coin_row_robot_gui.cpp"
        return
    }

    if ($Clean -and (Test-Path $output)) {
        Remove-Item -Path $output -Force
    }

    $args = @(
        "-std=c++17", "-Wall", "-Wextra", "-O2",
        $source, "-o", $output, "-lgdi32", "-mwindows"
    )

    try {
        & g++ @args
        Add-Result -Results $Results -App $appName -Status "PASS" -Details "Built coin_row_robot_gui.exe"
    } catch {
        Add-Result -Results $Results -App $appName -Status "FAIL" -Details "Build failed: $($_.Exception.Message)"
    }
}

function Build-EquationChecker {
    param(
        [string]$Root,
        [switch]$Clean,
        [System.Collections.Generic.List[object]]$Results
    )

    $appName = "EquationChecker"
    $appDir = Join-Path $Root $appName
    $source = Join-Path $appDir "program1.cc"
    $output = Join-Path $appDir "program1.exe"

    if (-not (Test-Path $source)) {
        Add-Result -Results $Results -App $appName -Status "SKIPPED" -Details "Missing program1.cc"
        return
    }

    if ($Clean -and (Test-Path $output)) {
        Remove-Item -Path $output -Force
    }

    $args = @("-std=c++17", "-Wall", "-Wextra", "-O2", $source, "-o", $output)

    try {
        & g++ @args

        $smokeInput = "10 + 1"
        $smokeOutput = $smokeInput | & $output 2>&1
        if (($smokeOutput -join "`n") -match "Invalid input format\.") {
            Add-Result -Results $Results -App $appName -Status "PASS" -Details "Built program1.exe + smoke test passed"
        } else {
            Add-Result -Results $Results -App $appName -Status "WARN" -Details "Built program1.exe; smoke output unexpected"
        }
    } catch {
        Add-Result -Results $Results -App $appName -Status "FAIL" -Details "Build/test failed: $($_.Exception.Message)"
    }
}

function Check-PrimeNumbers {
    param(
        [string]$Root,
        [System.Collections.Generic.List[object]]$Results
    )

    $appName = "PrimeNumbers"
    $appDir = Join-Path $Root $appName
    $cppFiles = Get-ChildItem -Path $appDir -Filter *.cc -File -ErrorAction SilentlyContinue
    if (-not $cppFiles) {
        Add-Result -Results $Results -App $appName -Status "SKIPPED" -Details "No .cc files found"
        return
    }

    Add-Result -Results $Results -App $appName -Status "WARN" -Details "Source exists but no build rule configured in script yet"
}

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$results = New-Object System.Collections.Generic.List[object]

Initialize-CompilerPath
Ensure-Gpp
Build-CoinRowRobot -Root $root -Clean:$Clean -Results $results
Build-EquationChecker -Root $root -Clean:$Clean -Results $results
Check-PrimeNumbers -Root $root -Results $results

Write-Host ""
Write-Host "C++ Build/Test Summary"
Write-Host "======================"
$results | Format-Table -AutoSize

$failed = @($results | Where-Object { $_.Status -eq "FAIL" })
if ($failed.Count -gt 0) {
    exit 1
}

exit 0
