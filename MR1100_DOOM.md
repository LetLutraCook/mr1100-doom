# DOOM on the Netgear MR1100

Run Freedoom/DOOM on the Netgear Nighthawk M1 (MR1100) using its built-in 320×240 LCD, with keyboard controls sent wirelessly from a Windows PC over the LAN.

## 1. Hardware and software

### Router

| Component | Details |
|---|---|
| Device | Netgear Nighthawk MR1100 |
| SoC | Qualcomm MDM9650 |
| CPU | ARM Cortex-A7 |
| Architecture | ARMv7, `armv7l` |
| OS | Linux 3.18.31 |
| Firmware | NTG9X50C_12.05.05.00 |
| Display controller | ST7789S |
| Framebuffer | `/dev/fb0` |
| Resolution | 320×240 |
| Color depth | 16-bit RGB565 |
| Stride | 640 bytes |

The router runs a lightweight embedded Linux environment with BusyBox.

### PC development environment

- Windows 11
- WSL2, Ubuntu 24.04
- ARM cross-compiler: `arm-linux-gnueabi-gcc`
- Python 3
- Doomgeneric source code
- Freedoom IWAD

Project location:

```text
C:\Users\lutra\doomgeneric
```

Source directory:

```text
C:\Users\lutra\doomgeneric\doomgeneric
```

## 2. Cross-compilation

The router uses ARMv7 with the softfp ABI. The correct compiler is:

```bash
arm-linux-gnueabi-gcc
```

Install it in WSL if necessary:

```bash
sudo apt update
sudo apt install -y gcc-arm-linux-gnueabi binutils-arm-linux-gnueabi
```

The project uses a custom Makefile:

```text
Makefile.mr1100
```

Important build settings:

- `-march=armv7-a`
- `-mfpu=neon`
- `-mfloat-abi=softfp`
- `-static`
- Resolution: 320×240

The resulting executable is statically linked, so it doesn't depend on additional libraries being installed on the router.

### Build command

From PowerShell:

```powershell
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/c/Users/lutra/doomgeneric/doomgeneric && make -f Makefile.mr1100 clean all"
```

Output executable:

```text
C:\Users\lutra\doomgeneric\doomgeneric\doomgeneric
```

## 3. Display implementation

The custom framebuffer backend is:

```text
doomgeneric_mr1100.c
```

It replaces the normal desktop graphics backend.

### Framebuffer configuration

The framebuffer reports:

```text
Resolution: 320x240
Virtual: 320x240
Stride: 640
Red: offset=11 length=5
Green: offset=5 length=6
Blue: offset=0 length=5
```

This is standard RGB565.

### Color inversion fix

The LCD initially displayed complementary colors:

| Expected | Initially displayed |
|---|---|
| Red | Cyan |
| Green | Magenta |
| Blue | Yellow |

The fix was to invert the final 16-bit RGB565 pixel:

```c
dst[x] = (uint16_t)~(
    ((r >> (8 - vi.red.length)) << vi.red.offset) |
    ((g >> (8 - vi.green.length)) << vi.green.offset) |
    ((b >> (8 - vi.blue.length)) << vi.blue.offset));
```

This corrected the colors.

### Vertical scaling

DOOM renders at 320×200, but the LCD is 320×240.

We stretch the rendered image vertically to fill the display:

```c
for (int y = 0; y < 240; y++) {
    int sy = y * 200 / 240;
```

The source pixel must use `sy`, not `y`:

```c
uint32_t p = src[sy * 320 + x];
```

This scales the original 200 rows across all 240 display rows.

The framebuffer is also cleared during initialization to prevent uninitialized pixels from appearing as a white strip:

```c
memset(fb, 0, fi.smem_len);
memset(DG_ScreenBuffer, 0, 320 * 240 * sizeof(uint32_t));
```

## 4. Keyboard networking

Keyboard input is sent from the Windows PC to the MR1100 using UDP.

The router listens on:

```text
UDP port 23232
```

The custom backend initializes a nonblocking UDP socket in `DG_Init()`.

`DG_GetKey()` receives two-byte messages:

| Byte | Meaning |
|---|---|
| 0 | Key released (0) or pressed (1) |
| 1 | Doom key code |

The backend returns these events to Doomgeneric, which posts the corresponding key-down and key-up events.

### Current keyboard bindings

| PC key | Doom action |
|---|---|
| W | Move forward |
| S | Move backward |
| A | Turn left |
| D | Turn right |
| Space | Use |
| Ctrl | Fire |
| Escape | Menu |
| Enter | Confirm |

The sender uses Windows `GetAsyncKeyState()` to detect held keys and sends UDP messages only when their state changes.

Mouse aiming is not implemented yet.

## 5. One-time setup

### Install Freedoom

In WSL:

```bash
sudo apt update
sudo apt install -y freedoom
```

The IWAD used for this project is:

```text
/usr/share/games/doom/freedoom1.wad
```

Copy it into the project directory:

```bash
cp /usr/share/games/doom/freedoom1.wad /mnt/c/Users/lutra/doomgeneric/doomgeneric/
```

The router will store it at:

```text
/tmp/freedoom1.wad
```

**Important:** `/tmp` is temporary storage. After a router reboot, the WAD and executable must be transferred again.

### Create the Windows keyboard sender

The sender is saved as:

```text
C:\Users\lutra\doomgeneric\doomgeneric\mr1100_keys.py
```

It uses only Python's standard library and Windows keyboard APIs.

## 6. Running DOOM again

This is the normal startup procedure.

### Step 1: Start the HTTP server on Windows

Open PowerShell:

```powershell
cd C:\Users\lutra\doomgeneric\doomgeneric
python -m http.server 8000 --bind 0.0.0.0
```

Keep this window open.

The PC's current LAN address is:

```text
192.168.1.85
```

If that changes, update the download URLs in the router commands.

### Step 2: Transfer files to the router

Connect to the MR1100 root shell through Telnet.

Run:

```sh
wget -qO /tmp/doomgeneric http://192.168.1.85:8000/doomgeneric
chmod +x /tmp/doomgeneric
```

If the WAD isn't already present:

```sh
wget -qO /tmp/freedoom1.wad http://192.168.1.85:8000/freedoom1.wad
```

### Step 3: Launch DOOM

```sh
killall doomgeneric 2>/dev/null
/tmp/doomgeneric -iwad /tmp/freedoom1.wad
```

The game should now appear on the MR1100's LCD.

### Step 4: Start keyboard input

Open a second PowerShell window:

```powershell
python C:\Users\lutra\doomgeneric\doomgeneric\mr1100_keys.py 192.168.1.1
```

`192.168.1.1` is the router's assumed LAN address. Substitute its actual LAN IP if it changes.

The sender should print:

```text
Sending keys to 192.168.1.1:23232
```

You can now play using your PC keyboard.

Press `Ctrl+C` in the sender window to stop keyboard input.

## 7. Useful troubleshooting

### HTTP download hangs

Check that:

- The Windows HTTP server is still running.
- The PC is still reachable at `192.168.1.85`.
- Both devices are connected to the same LAN.
- Windows Firewall allows inbound TCP port 8000.

### Keyboard doesn't work

Check that:

- DOOM is running on the router.
- The sender is targeting the correct router IP.
- UDP port 23232 is reachable.
- Windows Firewall isn't blocking the sender.

The sender and Doom backend must use the same UDP port.

### Colors look inverted

Verify that the final RGB565 pixel conversion still contains:

```c
(uint16_t)~
```

Removing it reverses the color correction.

### White strip or incorrect vertical position

Verify that the renderer uses:

```c
int sy = y * 200 / 240;
```

And reads:

```c
src[sy * 320 + x]
```

Using `src[y * 320 + x]` defeats the scaling.

### Rebuild after changing the backend

```powershell
wsl -d Ubuntu-24.04 -- bash -c "cd /mnt/c/Users/lutra/doomgeneric/doomgeneric && make -f Makefile.mr1100 clean all"
```

Then restart the HTTP server if necessary and transfer the newly built executable to the router.

## 8. Project files

| File | Purpose |
|---|---|
| `doomgeneric_mr1100.c` | Custom framebuffer and UDP input backend |
| `Makefile.mr1100` | ARM cross-compilation configuration |
| `doomgeneric` | Compiled ARM executable |
| `freedoom1.wad` | Freedoom game data |
| `mr1100_keys.py` | Windows keyboard sender |
| `patch_input.py` | Script that adds UDP input to the backend |
| `fbinfo.c` | Framebuffer information diagnostic |
| `fbtest.c` | RGB565 color test |

## 9. Current status

- [x] DOOM runs natively on the MR1100.
- [x] ARMv7 cross-compilation works.
- [x] Static executable runs on the router.
- [x] Freedoom loads successfully.
- [x] LCD framebuffer output works.
- [x] RGB565 color inversion corrected.
- [x] 320×200 image stretched to 320×240.
- [x] White strip eliminated.
- [x] UDP keyboard input implemented.
- [x] Windows keyboard sender implemented.
- [ ] Mouse aiming.
- [ ] Sound output.
- [ ] Improved performance and rendering optimization.
- [ ] Automatic startup integration.

**Current result:** A rooted Netgear MR1100 running DOOM on its own LCD, controlled wirelessly using a Windows PC keyboard.
