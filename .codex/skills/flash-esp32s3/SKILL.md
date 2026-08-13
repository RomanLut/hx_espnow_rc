---
name: flash-esp32s3
description: Build, flash, recover, and verify the ESP32-S3 Super Mini MAVLink/ESP-NOW USB receiver in examples/rx_esp32s3_mavlink_rc_usb. Use when asked to compile or flash this S3, enter its ROM loader, recover it after an interrupted flash, identify its changing COM port, or verify USB CDC, MAVLink RC data, watchdog stability, and hot re-enumeration after flashing.
---

# Flash ESP32-S3

Use the bundled `scripts/flash.ps1` from the repository root. It implements the verified Windows workflow and fails instead of guessing a port.

## Workflow

1. Inspect repository changes and preserve unrelated work.
2. Apply the repository `AGENTS.md` PlatformIO serialization rules.
3. Build once, sequentially:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .codex/skills/flash-esp32s3/scripts/flash.ps1 -BuildOnly
   ```

4. Flash and verify:

   ```powershell
   powershell -ExecutionPolicy Bypass -File .codex/skills/flash-esp32s3/scripts/flash.ps1
   ```

5. Report the discovered application and ROM ports, hashes verified by esptool, enumeration stability, MAVLink frame count, and decoded RC channel ranges.

## Safety and diagnosis rules

- Identify the board by VID:PID `303A:1001`; prefer serial/MAC `AC276EB156A0` / `AC:27:6E:B1:56:A0`. Never assume a fixed COM number.
- Enter the loader automatically by opening application CDC at 1200 baud. The expected transition is application `MI_01` to ROM USB-Serial/JTAG `MI_00`; rediscover the port after re-enumeration.
- A serial exception while setting 1200 baud can mean the requested reset already occurred. Rediscover before declaring failure.
- If automatic entry fails, try the Arduino CDC DTR/RTS sequence once. Only then request the physical R/GPIO2 hold-during-reconnect procedure.
- Probe the ROM port with esptool `--before no-reset --after no-reset chip-id` before writing.
- Flash all four regions when the SDK, bootloader, partitions, or flash configuration changed. The helper does this by default.
- Keep the target configured for its physical 4 MB flash even when the generic board name reports N8/8 MB.
- If flashing is interrupted after erasure begins, treat the image as incomplete and repeat the complete four-region flash.
- Require `Hash of data verified` for every region and confirm the application remains enumerated for at least 15 seconds.
- Open application CDC with DTR and RTS asserted and parse complete MAVLink frames. Do not claim RC success from byte counts alone.
- Do not use resets, replugging, flushing, or arbitrary delays to disguise a CDC or data-path failure. Preserve and report the failing layer.

## RC verification interpretation

- `RC_CHANNELS_OVERRIDE` is MAVLink message 70.
- Channels 1-15 come from the received ESP-NOW channel payload; channel 16 is locally overwritten with link RSSI by this firmware.
- If all source channels are exactly 1000, decode and report that fact. Do not rewrite them to 1500 or blame the FC/Quest without inspecting the raw source payload.
- Channels 17-18 are zero-valued MAVLink 2 extension fields and may be omitted by trailing-zero truncation; an FC can display its own neutral default for them.
