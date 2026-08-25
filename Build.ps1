<#
.SYNOPSIS
    Configures and builds the engine.

.DESCRIPTION
    Requirements are stated in README.mde

.PARAMETER Configuration
    Debug (default) or Release.

.PARAMETER Target
    Build (default), Rebuild, Clean, or a specific CMake target such as Banks.

.PARAMETER Fresh
    Discard the CMake cache and reconfigure from scratch.
#>
param(
    [string] $Target = 'Build',
    [ValidateSet('Debug', 'Release')]
    [string] $Configuration = 'Debug',
    [switch] $Fresh
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$BuildDir = Join-Path $PSScriptRoot 'build'

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'cmake was not found on PATH. Install CMake 3.21 or newer.'
}

if (-not (Test-Path (Join-Path $PSScriptRoot 'External\SDL\CMakeLists.txt'))) {
    throw 'Submodules are missing. Run: git submodule update --init --recursive'
}

# Multi-config generator, so the configuration is chosen at build time, not here.
if ($Fresh -or -not (Test-Path (Join-Path $BuildDir 'CMakeCache.txt'))) {
    # -Wno-dev: the vendored dependencies emit developer warnings from their own
    # CMakeLists that we cannot fix and that drown out our own output.
    cmake -S $PSScriptRoot -B $BuildDir -A x64 -Wno-dev $(if ($Fresh) { '--fresh' })
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$extra = switch ($Target) {
    'Build'   { @() }
    'Rebuild' { @('--clean-first') }
    'Clean'   { @('--target', 'clean') }
    default   { @('--target', $Target) }
}

cmake --build $BuildDir --config $Configuration --parallel @extra

exit $LASTEXITCODE
