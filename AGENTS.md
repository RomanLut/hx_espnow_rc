# Agent Instructions

## ESP32 PlatformIO serialization

- Never run PlatformIO build, clean, or upload operations concurrently. They share packages, build outputs, and serial ports.
- Use `C:\Users\roman\.platformio\penv\Scripts\pio.exe`; do not use a global `pio` command.
- Before retrying a failed or timed-out operation, confirm that its PlatformIO, Python, SCons, esptool, CMake, Ninja, and compiler child processes have exited. Never overlap a retry with the original operation.
- Do not clean unless generated state is demonstrably invalid. Prefer an incremental build.

## ESP32-S3 MAVLink USB receiver flashing

For `examples/rx_esp32s3_mavlink_rc_usb`, use the project skill at
`.codex/skills/flash-esp32s3/SKILL.md`. It is the single source of truth for
building, bootloader entry, COM-port rediscovery, flashing, recovery, and
post-flash USB/MAVLink verification.
