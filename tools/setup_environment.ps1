# tools/setup_environment.ps1
# Helper script to download devkitPro installer and NUSPacker on Windows

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "    Wii U Homebrew Toolchain Setup Helper for Pou        " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host ""

# 1. Check NUSPacker
$nusPackerPath = Join-Path $PSScriptRoot "NUSPacker.jar"
if (Test-Path $nusPackerPath) {
    Write-Host "[OK] NUSPacker.jar is already present in tools/" -ForegroundColor Green
} else {
    Write-Host "[...] Downloading NUSPacker.jar..." -ForegroundColor Yellow
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 -bor [Net.SecurityProtocolType]::Tls13
    $url = "https://raw.githubusercontent.com/ihaveamac/nuspacker/master/NUSPacker.jar"
    try {
        $wc = New-Object System.Net.WebClient
        $wc.Headers.Add("User-Agent", "Mozilla/5.0")
        $wc.DownloadFile($url, $nusPackerPath)
        $wc.Dispose()
        Write-Host "[OK] NUSPacker.jar downloaded successfully!" -ForegroundColor Green
    } catch {
        Write-Warning "Could not download NUSPacker automatically: $_"
    }
}

# 2. Check Java
$javaCmd = Get-Command java -ErrorAction SilentlyContinue
if ($javaCmd) {
    Write-Host "[OK] Java runtime found: $((Get-Command java).Source)" -ForegroundColor Green
} else {
    Write-Host "[!] Java runtime not found (required for NUSPacker)." -ForegroundColor Yellow
    Write-Host "    You can install lightweight OpenJDK via winget:" -ForegroundColor Gray
    Write-Host "    winget install EclipseAdoptium.Temurin.17.JRE" -ForegroundColor Cyan
}

# 3. Check devkitPro
$hasDevkitPro = $false
if ($env:DEVKITPRO -and (Test-Path $env:DEVKITPRO)) {
    $hasDevkitPro = $true
} elseif (Test-Path "C:\devkitPro") {
    $hasDevkitPro = $true
}

if ($hasDevkitPro) {
    Write-Host "[OK] devkitPro detected!" -ForegroundColor Green
} else {
    Write-Host "[!] devkitPro / WUT not found." -ForegroundColor Yellow
    Write-Host "    To install devkitPro on Windows:" -ForegroundColor Gray
    Write-Host "    1. Download the installer: https://github.com/devkitPro/installer/releases/latest" -ForegroundColor Cyan
    Write-Host "    2. Select 'Wii U Development' during setup." -ForegroundColor Gray
    Write-Host "    3. Open C:\devkitPro\msys2\msys2.exe and run:" -ForegroundColor Gray
    Write-Host "       pacman -S devkitPPC wut wiiu-portlibs wiiu-sdl2 wiiu-sdl2_image wiiu-sdl2_ttf wiiu-sdl2_mixer" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "----------------------------------------------------------" -ForegroundColor DarkGray
Write-Host "TIP: If you don't want to install anything locally, you can" -ForegroundColor White
Write-Host "     use GitHub Actions! A ready-to-run workflow is in" -ForegroundColor White
Write-Host "     .github/workflows/build.yml which compiles Pou.rpx" -ForegroundColor White
Write-Host "     and packages the WUP release in the cloud for free." -ForegroundColor White
Write-Host "----------------------------------------------------------" -ForegroundColor DarkGray
