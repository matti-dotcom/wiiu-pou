# tools/generate_meta_assets.ps1
# Generates Wii U Channel meta assets: iconTex.tga, bootTvTex.tga, bootDrcTex.tga

Add-Type -AssemblyName System.Drawing

function Save-TGA {
    param(
        [System.Drawing.Bitmap]$bitmap,
        [string]$outputPath,
        [int]$bpp = 32
    )
    $w = $bitmap.Width
    $h = $bitmap.Height
    $stream = [System.IO.File]::Create($outputPath)
    $writer = New-Object System.IO.BinaryWriter($stream)

    # 18-byte uncompressed true-color TGA header
    $header = New-Object byte[] 18
    $header[2] = 2 # Uncompressed RGB / RGBA
    $header[12] = [byte]($w -band 0xFF)
    $header[13] = [byte](($w -shr 8) -band 0xFF)
    $header[14] = [byte]($h -band 0xFF)
    $header[15] = [byte](($h -shr 8) -band 0xFF)
    $header[16] = [byte]$bpp
    # Top-to-bottom: descriptor bit 5 (0x20). 32-bit has 8 alpha bits (0x08), total 0x28
    $header[17] = if ($bpp -eq 32) { [byte]0x28 } else { [byte]0x20 }
    $writer.Write($header)

    $rect = New-Object System.Drawing.Rectangle(0, 0, $w, $h)
    $format = if ($bpp -eq 32) { [System.Drawing.Imaging.PixelFormat]::Format32bppArgb } else { [System.Drawing.Imaging.PixelFormat]::Format24bppRgb }
    $bmpData = $bitmap.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, $format)

    $numBytes = [Math]::Abs($bmpData.Stride) * $h
    $bytes = New-Object byte[] $numBytes
    [System.Runtime.InteropServices.Marshal]::Copy($bmpData.Scan0, $bytes, 0, $numBytes)
    $bitmap.UnlockBits($bmpData)

    $rowSize = $w * ($bpp / 8)
    for ($y = 0; $y -lt $h; $y++) {
        $offset = $y * $bmpData.Stride
        $writer.Write($bytes, $offset, $rowSize)
    }

    $writer.Close()
    $stream.Close()
    Write-Host "Generated: $outputPath ($w x $h, $bpp-bit TGA)"
}

$baseDir = Split-Path -Parent $PSScriptRoot
$metaDir = Join-Path $baseDir "meta"
$assetsDir = Join-Path $baseDir "content\assets"

# Load font
$pfc = New-Object System.Drawing.Text.PrivateFontCollection
$fontPath = Join-Path $assetsDir "fonts\Cookies.ttf"
$pfc.AddFontFile($fontPath)
$cookiesFamily = $pfc.Families[0]

# Helper to draw procedural Pou
function Draw-Pou {
    param(
        [System.Drawing.Graphics]$g,
        [float]$centerX,
        [float]$centerY,
        [float]$scale
    )
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias

    # Body geometry: teardrop rounded triangle
    $bodyW = 160.0 * $scale
    $bodyH = 140.0 * $scale
    $pouRect = New-Object System.Drawing.RectangleF ($centerX - $bodyW/2), ($centerY - $bodyH/2), $bodyW, $bodyH

    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    # Smooth Pou rounded body
    $path.AddArc(($centerX - $bodyW*0.48), ($centerY + $bodyH*0.1), ($bodyW*0.35), ($bodyH*0.35), 90, 90)
    $path.AddArc(($centerX - $bodyW*0.3), ($centerY - $bodyH*0.45), ($bodyW*0.6), ($bodyH*0.5), 180, 180)
    $path.AddArc(($centerX + $bodyW*0.13), ($centerY + $bodyH*0.1), ($bodyW*0.35), ($bodyH*0.35), 0, 90)
    $path.CloseFigure()

    $pouBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(199, 149, 90)) # Classic Pou Brown
    $pouOutline = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(120, 80, 40), (5.0 * $scale))
    $pouOutline.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round

    $g.FillPath($pouBrush, $path)
    $g.DrawPath($pouOutline, $path)

    # Eyes
    $eyeW = 38.0 * $scale
    $eyeH = 46.0 * $scale
    $eyeSpacing = 28.0 * $scale
    $eyeY = $centerY - (12.0 * $scale)

    $eyeBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::White)
    $eyeOutline = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(40, 40, 40), (3.0 * $scale))

    # Left eye
    $leftEyeRect = New-Object System.Drawing.RectangleF ($centerX - $eyeSpacing - $eyeW), $eyeY, $eyeW, $eyeH
    $g.FillEllipse($eyeBrush, $leftEyeRect)
    $g.DrawEllipse($eyeOutline, $leftEyeRect)

    # Right eye
    $rightEyeRect = New-Object System.Drawing.RectangleF ($centerX + $eyeSpacing), $eyeY, $eyeW, $eyeH
    $g.FillEllipse($eyeBrush, $rightEyeRect)
    $g.DrawEllipse($eyeOutline, $rightEyeRect)

    # Pupils
    $pupilW = 16.0 * $scale
    $pupilH = 18.0 * $scale
    $pupilBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(40, 40, 40))
    $shineBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::White)

    $leftPupil = New-Object System.Drawing.RectangleF ($centerX - $eyeSpacing - $eyeW*0.65), ($eyeY + $eyeH*0.35), $pupilW, $pupilH
    $rightPupil = New-Object System.Drawing.RectangleF ($centerX + $eyeSpacing + $eyeW*0.25), ($eyeY + $eyeH*0.35), $pupilW, $pupilH
    $g.FillEllipse($pupilBrush, $leftPupil)
    $g.FillEllipse($pupilBrush, $rightPupil)

    # Pupil shine
    $shineSize = 5.0 * $scale
    $leftShine = New-Object System.Drawing.RectangleF ($leftPupil.X + 2*$scale), ($leftPupil.Y + 2*$scale), $shineSize, $shineSize
    $rightShine = New-Object System.Drawing.RectangleF ($rightPupil.X + 2*$scale), ($rightPupil.Y + 2*$scale), $shineSize, $shineSize
    $g.FillEllipse($shineBrush, $leftShine)
    $g.FillEllipse($shineBrush, $rightShine)

    # Smile / Mouth
    $mouthPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(50, 30, 20), (3.5 * $scale))
    $mouthPen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $mouthPen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $mouthRect = New-Object System.Drawing.RectangleF ($centerX - 18*$scale), ($centerY + 24*$scale), (36*$scale), (20*$scale)
    $g.DrawArc($mouthPen, $mouthRect, 20, 140)

    # Cleanup
    $path.Dispose()
    $pouBrush.Dispose()
    $pouOutline.Dispose()
    $eyeBrush.Dispose()
    $eyeOutline.Dispose()
    $pupilBrush.Dispose()
    $shineBrush.Dispose()
    $mouthPen.Dispose()
}

# 1. Generate iconTex.tga (128x128 RGBA 32-bit)
$iconBmp = New-Object System.Drawing.Bitmap 128, 128
$gIcon = [System.Drawing.Graphics]::FromImage($iconBmp)
$gIcon.Clear([System.Drawing.Color]::FromArgb(250, 240, 215)) # Warm cream
Draw-Pou $gIcon 64 68 0.65
$gIcon.Dispose()
Save-TGA $iconBmp (Join-Path $metaDir "iconTex.tga") 32
$iconBmp.Dispose()

# 2. Generate bootTvTex.tga (1920x1080 RGB 24-bit)
$tvBmp = New-Object System.Drawing.Bitmap 1920, 1080
$gTv = [System.Drawing.Graphics]::FromImage($tvBmp)
# Wallpaper gradient
$linGrad = New-Object System.Drawing.Drawing2D.LinearGradientBrush (
    (New-Object System.Drawing.PointF 0, 0),
    (New-Object System.Drawing.PointF 0, 1080),
    ([System.Drawing.Color]::FromArgb(255, 248, 230)),
    ([System.Drawing.Color]::FromArgb(235, 215, 185))
)
$gTv.FillRectangle($linGrad, 0, 0, 1920, 1080)
$linGrad.Dispose()

# Draw big Pou in center
Draw-Pou $gTv 960 560 3.8

# Draw "Pou" title in Cookies font
$fontTitle = New-Object System.Drawing.Font ($cookiesFamily, 84, [System.Drawing.FontStyle]::Bold)
$titleBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(85, 55, 30))
$sf = New-Object System.Drawing.StringFormat
$sf.Alignment = [System.Drawing.StringAlignment]::Center
$sf.LineAlignment = [System.Drawing.StringAlignment]::Center
$gTv.DrawString("Pou", $fontTitle, $titleBrush, 960, 180, $sf)

# Subtitle
$fontSub = New-Object System.Drawing.Font ($cookiesFamily, 32, [System.Drawing.FontStyle]::Regular)
$subBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(140, 110, 80))
$gTv.DrawString("for Nintendo Wii U", $fontSub, $subBrush, 960, 260, $sf)

$fontTitle.Dispose()
$titleBrush.Dispose()
$fontSub.Dispose()
$subBrush.Dispose()
$sf.Dispose()
$gTv.Dispose()
Save-TGA $tvBmp (Join-Path $metaDir "bootTvTex.tga") 24
$tvBmp.Dispose()

# 3. Generate bootDrcTex.tga (854x480 RGB 24-bit)
$drcBmp = New-Object System.Drawing.Bitmap 854, 480
$gDrc = [System.Drawing.Graphics]::FromImage($drcBmp)
$linGradDrc = New-Object System.Drawing.Drawing2D.LinearGradientBrush (
    (New-Object System.Drawing.PointF 0, 0),
    (New-Object System.Drawing.PointF 0, 480),
    ([System.Drawing.Color]::FromArgb(255, 248, 230)),
    ([System.Drawing.Color]::FromArgb(235, 215, 185))
)
$gDrc.FillRectangle($linGradDrc, 0, 0, 854, 480)
$linGradDrc.Dispose()

# Draw Pou on GamePad
Draw-Pou $gDrc 427 250 1.9

# Title
$fontTitleDrc = New-Object System.Drawing.Font ($cookiesFamily, 44, [System.Drawing.FontStyle]::Bold)
$titleBrushDrc = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(85, 55, 30))
$sfDrc = New-Object System.Drawing.StringFormat
$sfDrc.Alignment = [System.Drawing.StringAlignment]::Center
$sfDrc.LineAlignment = [System.Drawing.StringAlignment]::Center
$gDrc.DrawString("Pou", $fontTitleDrc, $titleBrushDrc, 427, 75, $sfDrc)

$fontTitleDrc.Dispose()
$titleBrushDrc.Dispose()
$sfDrc.Dispose()
$gDrc.Dispose()
Save-TGA $drcBmp (Join-Path $metaDir "bootDrcTex.tga") 24
$drcBmp.Dispose()

$pfc.Dispose()
Write-Host "All meta assets generated successfully!"
