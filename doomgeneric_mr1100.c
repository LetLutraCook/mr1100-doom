#include "doomgeneric.h"
#include "i_system.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int fd;
static uint8_t *fb;
static struct fb_var_screeninfo vi;
static struct fb_fix_screeninfo fi;
static struct timeval start;
static int input_sock = -1;

void DG_Init(void) {
    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) I_Error("Cannot open framebuffer");

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0 ||
        ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0)
        I_Error("Cannot read framebuffer info");

    if (vi.xres != 320 || vi.yres != 240 || vi.bits_per_pixel != 16)
        I_Error("Unexpected framebuffer format");

    fb = mmap(NULL, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb == MAP_FAILED) I_Error("Framebuffer mmap failed");

    memset(fb, 0, fi.smem_len);
    memset(DG_ScreenBuffer, 0, 320 * 240 * sizeof(uint32_t));
    input_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (input_sock < 0) I_Error("UDP socket failed");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(23232);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(input_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        I_Error("UDP bind failed");

    fcntl(input_sock, F_SETFL, O_NONBLOCK);
    gettimeofday(&start, NULL);
}

void DG_DrawFrame(void) {
    uint32_t *src = (uint32_t *)DG_ScreenBuffer;

    for (int y = 0; y < 240; y++) {
        int sy = y * 200 / 240;
        uint16_t *dst = (uint16_t *)(fb + y * fi.line_length);

        for (int x = 0; x < 320; x++) {
            uint32_t p = src[sy * 320 + x];
            uint32_t r = (p >> 16) & 255;
            uint32_t g = (p >> 8) & 255;
            uint32_t b = p & 255;

            dst[x] = (uint16_t)~(
                ((r >> (8 - vi.red.length)) << vi.red.offset) |
                ((g >> (8 - vi.green.length)) << vi.green.offset) |
                ((b >> (8 - vi.blue.length)) << vi.blue.offset));
        }
    }
}

void DG_SleepMs(uint32_t ms) {
    usleep(ms * 1000);
}

uint32_t DG_GetTicksMs(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec - start.tv_sec) * 1000 +
           (t.tv_usec - start.tv_usec) / 1000;
}

int DG_GetKey(int *pressed, unsigned char *key) {
    unsigned char msg[2];
    int n = recv(input_sock, msg, 2, MSG_DONTWAIT);
    if (n != 2) return 0;
    *pressed = msg[0];
    *key = msg[1];
    return 1;
}

void DG_SetWindowTitle(const char *title) {}

int main(int argc, char **argv) {
    doomgeneric_Create(argc, argv);
    for (;;) doomgeneric_Tick();
}
