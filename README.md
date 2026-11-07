# DOOM on the Netgear MR1100

## ⚠️ Project is vibecoded!

A custom [Doomgeneric](https://github.com/ozkl/doomgeneric) backend that runs DOOM/Freedoom on the built-in 320×240 LCD of the Netgear Nighthawk M1 (MR1100).

The MR1100 runs the game locally. A small Windows Python script sends keyboard events to the router over UDP.

## What works

- Direct rendering to `/dev/fb0` (RGB565, 320×240)
- Corrected LCD color inversion
- Vertical scaling from Doomgeneric's 320×200 frame to the LCD's 320×240 display
- Keyboard input over UDP port `23232`
- ARMv7 cross-compilation with a statically linked executable

Mouse aiming, sound, performance optimization, and automatic startup are not implemented.

## Repository contents

| File | Purpose |
|---|---|
| `doomgeneric_mr1100.c` | MR1100 framebuffer renderer and UDP input backend |
| `Makefile.mr1100` | ARM cross-compilation Makefile |
| `mr1100_keys.py` | Windows keyboard-to-UDP sender |
| `patch_input.py` | Script for applying the UDP input changes to the backend |
| `MR1100_DOOM.md` | Full hardware notes, setup, build, run, and troubleshooting guide |

**This repository contains the MR1100-specific port files, not the complete Doomgeneric engine or game data.** Start with a separate Doomgeneric source checkout, then place these files alongside its source files. The Makefile expects the normal Doomgeneric source tree.

## Build requirements

- Netgear Nighthawk M1 (MR1100) with root access
- Windows PC on the same LAN as the router
- WSL2 with Ubuntu 24.04
- `arm-linux-gnueabi-gcc` and `binutils-arm-linux-gnueabi`
- Python 3 on Windows
- A legally obtained DOOM IWAD, or [Freedoom](https://freedoom.github.io/)

Install the cross-compiler in WSL:

```bash
sudo apt update
sudo apt install -y gcc-arm-linux-gnueabi binutils-arm-linux-gnueabi
```

Build from the Doomgeneric source directory after adding this repository's files:

```bash
make -f Makefile.mr1100 clean all
```

The output is a statically linked ARM executable named `doomgeneric`.

## Quick links

- [Full setup and usage guide](MR1100_DOOM.md)
- [Framebuffer backend](doomgeneric_mr1100.c)
- [Build configuration](Makefile.mr1100)

## Notes

The MR1100's `/tmp` directory is temporary storage. The executable and IWAD must be transferred again after a router reboot unless you choose another storage location. Do not commit copyrighted IWAD files, compiled binaries, router credentials, or other private configuration.
