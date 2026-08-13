param(
    [switch]$BuildOnly,
    [switch]$SkipBuild,
    [int]$StabilitySeconds = 15,
    [int]$CaptureSeconds = 5
)

$ErrorActionPreference = 'Stop'
$projectDirectory = Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..\examples\rx_esp32s3_mavlink_rc_usb')
$pioExecutable = 'C:\Users\roman\.platformio\penv\Scripts\pio.exe'
$pythonExecutable = 'C:\pio\penv\Scripts\python.exe'
$buildDirectory = Join-Path $projectDirectory '.pio\build\esp32-s3-devkitc-1'
$targetSerial = 'AC276EB156A0'
$env:PLATFORMIO_CORE_DIR = 'C:\pio'
$env:PYTHONUTF8 = '1'

function Get-EspressifPorts
{
    @(Get-CimInstance Win32_SerialPort | Where-Object { $_.PNPDeviceID -match 'VID_303A&PID_1001' })
}

function Wait-EspressifPort
{
    param([string]$Interface, [int]$TimeoutSeconds = 10)

    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    do
    {
        $port = Get-EspressifPorts | Where-Object {
            $_.PNPDeviceID -match "&$Interface\\" -and
            ($_.PNPDeviceID -replace '[^A-Fa-f0-9]', '') -match $targetSerial
        } | Select-Object -First 1
        if (-not $port)
        {
            $port = Get-EspressifPorts | Where-Object { $_.PNPDeviceID -match "&$Interface\\" } | Select-Object -First 1
        }
        if ($port)
        {
            return $port.DeviceID
        }
        Start-Sleep -Milliseconds 200
    } while ([DateTime]::UtcNow -lt $deadline)

    throw "Timed out waiting for Espressif $Interface port."
}

$busyProcesses = Get-Process -ErrorAction SilentlyContinue | Where-Object {
    $_.ProcessName -match '^(platformio|pio|scons|esptool|cmake|ninja|xtensa.*)$'
}
if ($busyProcesses)
{
    throw "A firmware tool is already running: $($busyProcesses.ProcessName -join ', ')"
}

if (-not $SkipBuild)
{
    & $pioExecutable run --project-dir $projectDirectory
    if ($LASTEXITCODE -ne 0)
    {
        throw "PlatformIO build failed with exit code $LASTEXITCODE."
    }
}
if ($BuildOnly)
{
    exit 0
}

$applicationPort = Wait-EspressifPort -Interface 'MI_01'
Write-Output "Application CDC: $applicationPort"

$resetScript = @'
import serial, sys, time
try:
    port = serial.Serial(sys.argv[1], 1200, timeout=0.1)
    time.sleep(0.2)
    port.close()
except serial.SerialException:
    # Windows can fail SetCommState because the requested reset already detached CDC.
    pass
'@
$resetScript | & $pythonExecutable - $applicationPort
$romPort = Wait-EspressifPort -Interface 'MI_00'
Write-Output "ROM loader: $romPort"

& $pythonExecutable -m esptool --chip esp32s3 --port $romPort --before no-reset --after no-reset chip-id
if ($LASTEXITCODE -ne 0)
{
    throw 'ROM loader probe failed; flash was not attempted.'
}

& $pythonExecutable -m esptool --chip esp32s3 --port $romPort --baud 921600 `
    --before no-reset --after hard-reset write-flash -z `
    --flash-mode dio --flash-freq 80m --flash-size detect `
    0x0000 (Join-Path $buildDirectory 'bootloader.bin') `
    0x8000 (Join-Path $buildDirectory 'partitions.bin') `
    0xe000 'C:\pio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin' `
    0x10000 (Join-Path $buildDirectory 'firmware.bin')
if ($LASTEXITCODE -ne 0)
{
    throw "esptool write failed with exit code $LASTEXITCODE."
}

$applicationPort = Wait-EspressifPort -Interface 'MI_01'
Write-Output "Application returned: $applicationPort"
$deadline = [DateTime]::UtcNow.AddSeconds($StabilitySeconds)
while ([DateTime]::UtcNow -lt $deadline)
{
    if (-not (Get-EspressifPorts | Where-Object { $_.DeviceID -eq $applicationPort -and $_.PNPDeviceID -match '&MI_01\\' }))
    {
        throw "Application CDC $applicationPort disconnected during the stability interval."
    }
    Start-Sleep -Milliseconds 250
}
Write-Output "Application CDC stable for $StabilitySeconds seconds."

$captureScript = @'
import serial, sys, time
port, seconds = sys.argv[1], int(sys.argv[2])
s = serial.Serial()
s.port = port
s.baudrate = 115200
s.timeout = 0.1
s.dtr = True
s.rts = True
s.open()
data = bytearray()
try:
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        data.extend(s.read(4096))
finally:
    s.close()
mins, maxs, frames, offset = [65535] * 16, [0] * 16, 0, 0
while offset + 12 <= len(data):
    if data[offset] != 0xFD:
        offset += 1
        continue
    payload_len = data[offset + 1]
    frame_len = 12 + payload_len + (13 if data[offset + 2] & 1 else 0)
    if offset + frame_len > len(data):
        break
    message_id = data[offset + 7] | data[offset + 8] << 8 | data[offset + 9] << 16
    if message_id == 70 and payload_len >= 34:
        for channel in range(16):
            field_offset = channel * 2 if channel < 8 else 18 + (channel - 8) * 2
            value = int.from_bytes(data[offset + 10 + field_offset:offset + 12 + field_offset], 'little')
            mins[channel] = min(mins[channel], value)
            maxs[channel] = max(maxs[channel], value)
        frames += 1
    offset += frame_len
print(f'MAVLink bytes={len(data)} RC_CHANNELS_OVERRIDE frames={frames}')
if frames:
    print('RC min=' + ','.join(map(str, mins)))
    print('RC max=' + ','.join(map(str, maxs)))
else:
    raise SystemExit(3)
'@
$captureScript | & $pythonExecutable - $applicationPort $CaptureSeconds
if ($LASTEXITCODE -ne 0)
{
    throw 'No complete RC_CHANNELS_OVERRIDE frames were verified.'
}
