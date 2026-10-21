# tools/convert_boot_sound.ps1
# Converts Pou sound/music to Wii U bootSound.btsnd using wav2btsnd
param(
    [string]$InputWav = "..\content\assets\musics\music_food.wav",
    [string]$OutputBtsnd = "..\meta\bootSound.btsnd"
)

$wav2btsnd = Get-Command wav2btsnd -ErrorAction SilentlyContinue
if (-not $wav2btsnd) {
    # Check devkitPro tools
    if ($env:DEVKITPRO -and (Test-Path "$env:DEVKITPRO\tools\bin\wav2btsnd.exe")) {
        $wav2btsnd = "$env:DEVKITPRO\tools\bin\wav2btsnd.exe"
    } elseif (Test-Path "C:\devkitPro\tools\bin\wav2btsnd.exe") {
        $wav2btsnd = "C:\devkitPro\tools\bin\wav2btsnd.exe"
    }
}

if ($wav2btsnd) {
    Write-Host "Running wav2btsnd on $InputWav -> $OutputBtsnd"
    & $wav2btsnd -i $InputWav -o $OutputBtsnd -l 0
} else {
    Write-Warning "wav2btsnd tool not found in PATH or devkitPro. Run:"
    Write-Host "  wav2btsnd -i content/assets/musics/music_food.wav -o meta/bootSound.btsnd -l 0"
}
