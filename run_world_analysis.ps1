param(
    # Default input map file. You can override with: -MapFile <path>
    [string]$MapFile = ".\example\3_world_analysis\cases\case5_8x8.txt",
    [switch]$Rebuild
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $RepoRoot

$CondaRoot = "C:\Users\19310\anaconda3"
$ToolchainPath = @(
    "$CondaRoot\Library\usr\bin",
    "$CondaRoot\Library\mingw-w64\bin",
    "$CondaRoot\Library\bin"
)
$env:PATH = ($ToolchainPath -join ";") + ";" + $env:PATH

$Compiler = "$CondaRoot\Library\bin\x86_64-w64-mingw32-g++.exe"
$OutputExe = Join-Path $RepoRoot "world_analysis.exe"

if (!(Test-Path $Compiler)) {
    throw "Compiler not found: $Compiler"
}

if ($Rebuild -or !(Test-Path $OutputExe)) {
    Write-Host "[build] compiling world_analysis.exe ..."
    & $Compiler `
        -std=c++17 `
        -O2 `
        -I src/core/include `
        example/3_world_analysis/main.cpp `
        src/core/src/world/analysis.cpp `
        src/core/src/world/model.cpp `
        src/core/src/world/connectivity.cpp `
        src/core/src/utils/default_naming.cpp `
        src/core/src/utils/random_tool.cpp `
        -o world_analysis.exe

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
}

$ResolvedMap = Resolve-Path $MapFile -ErrorAction Stop
Write-Host "[run] map = $ResolvedMap"
& $OutputExe $ResolvedMap

if ($LASTEXITCODE -ne 0) {
    throw "Program exited with code $LASTEXITCODE"
}
