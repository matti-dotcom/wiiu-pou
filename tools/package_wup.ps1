# tools/package_wup.ps1
# Packages the Wii U title directory using NUSPacker for WUP Installer GX2
param(
    [string]$NusPackerJar = "NUSPacker.jar",
    [string]$InputDir = "..",
    [string]$OutputDir = "..\Pou_WUP_Output",
    [string]$CommonKey = $env:WIIU_COMMON_KEY
)

if (-not $CommonKey) {
    Write-Warning "WIIU_COMMON_KEY environment variable is not set."
    Write-Host "Please pass -CommonKey <32-char hex key> or set $env:WIIU_COMMON_KEY"
    exit 1
}

if (-not (Test-Path $NusPackerJar)) {
    Write-Error "NUSPacker.jar not found at: $NusPackerJar"
    Write-Host "Download NUSPacker from: https://github.com/FDG-Key/NUSPacker"
    exit 1
}

Write-Host "Packaging Wii U Title with NUSPacker..."
Write-Host "Input:  $InputDir"
Write-Host "Output: $OutputDir"

java -jar $NusPackerJar -in "$InputDir" -out "$OutputDir" -key "$CommonKey"
Write-Host "WUP Package generated successfully in: $OutputDir"
