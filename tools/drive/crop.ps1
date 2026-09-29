# crop.ps1 <in> <out> x y w h zoom : cut a region out of a screenshot and enlarge it with hard pixels
param([string]$in, [string]$out, [int]$x, [int]$y, [int]$w, [int]$h, [int]$zoom = 4)
Add-Type -AssemblyName System.Drawing
$src = [System.Drawing.Image]::FromFile($in)
$dst = New-Object System.Drawing.Bitmap ($w * $zoom), ($h * $zoom)
$g = [System.Drawing.Graphics]::FromImage($dst)
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
$g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
$g.DrawImage($src, (New-Object System.Drawing.Rectangle 0, 0, ($w * $zoom), ($h * $zoom)), (New-Object System.Drawing.Rectangle $x, $y, $w, $h), [System.Drawing.GraphicsUnit]::Pixel)
$dst.Save($out); $g.Dispose(); $dst.Dispose(); $src.Dispose()
