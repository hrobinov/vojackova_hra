# Drive the game: drive.ps1 start | shot <name> | move x y [name] | click x y [name] | keys <SendKeys text> [name]
# (x,y relative to the window's client area; keys uses System.Windows.Forms.SendKeys syntax, e.g. "^a50{ENTER}")
param([string]$cmd, [string]$a, [string]$b, [string]$name = 'last')
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public static class W {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  [StructLayout(LayoutKind.Sequential)] public struct PT { public int X, Y; }
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref PT p);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(int f, int x, int y, int d, int e);
  [DllImport("user32.dll")] public static extern void keybd_event(byte k, byte s, int f, int e);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
}
"@
[W]::SetProcessDPIAware() | Out-Null
$dir = Join-Path $env:TEMP "vojackova_hra-drive"; New-Item -ItemType Directory -Force $dir | Out-Null
if ($cmd -eq 'start') {
  Get-Process vojackova_hra -ErrorAction SilentlyContinue | Stop-Process -Force
  Start-Process 'C:\projekty\vojackova_hra\build\Debug\vojackova_hra.exe' -WorkingDirectory 'C:\projekty\vojackova_hra'
  Start-Sleep -Seconds 3
  $name = 'start'
}
$p = Get-Process vojackova_hra | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
$h = $p.MainWindowHandle

# Windows only lets the process that had the last input take the foreground: a tap of Alt makes that us.
for ($i = 0; $i -lt 10 -and [W]::GetForegroundWindow() -ne $h; $i++) {
  [W]::keybd_event(0x12, 0, 0, 0); [W]::keybd_event(0x12, 0, 2, 0)
  [W]::SetForegroundWindow($h) | Out-Null
  Start-Sleep -Milliseconds 200
}
if ([W]::GetForegroundWindow() -ne $h) { "could not bring the window to the front"; exit 1 }

# Wait for the size to settle (windowed fullscreen grows back on focus).
$r = New-Object W+RECT; $last = -1
for ($i = 0; $i -lt 20; $i++) {
  Start-Sleep -Milliseconds 150
  [W]::GetClientRect($h, [ref]$r) | Out-Null
  if ($r.R -eq $last) { break }
  $last = $r.R
}
Start-Sleep -Milliseconds 300
$o = New-Object W+PT; [W]::ClientToScreen($h, [ref]$o) | Out-Null

if ($cmd -eq 'move' -or $cmd -eq 'click') {
  [W]::SetCursorPos($o.X + [int]$a, $o.Y + [int]$b) | Out-Null
  Start-Sleep -Milliseconds 200
  if ($cmd -eq 'click') { [W]::mouse_event(2,0,0,0,0); Start-Sleep -Milliseconds 60; [W]::mouse_event(4,0,0,0,0) }
  Start-Sleep -Milliseconds 500
}
if ($cmd -eq 'keys') {
  Add-Type -AssemblyName System.Windows.Forms
  [System.Windows.Forms.SendKeys]::SendWait($a)
  Start-Sleep -Milliseconds 400
  if ($b) { $name = $b }
}
if ($cmd -eq 'shot') { $name = $a }
$w = $r.R; $hh = $r.B
$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($o.X, $o.Y, 0, 0, (New-Object System.Drawing.Size $w, $hh))
$out = Join-Path $dir "$name.png"
$bmp.Save($out); $g.Dispose(); $bmp.Dispose()
"client ${w}x${hh} at $($o.X),$($o.Y) -> $out"
