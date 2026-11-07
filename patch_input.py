p = 'doomgeneric_mr1100.c'
s = open(p).read()

s = s.replace(
    '#include <linux/fb.h>',
    '#include <linux/fb.h>\n#include <sys/socket.h>\n#include <netinet/in.h>'
)

s = s.replace(
    'static struct timeval start;',
    'static struct timeval start;\nstatic int input_sock = -1;'
)

s = s.replace(
    '    gettimeofday(&start, NULL);',
    '''    input_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (input_sock < 0) I_Error("UDP socket failed");

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(23232);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(input_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        I_Error("UDP bind failed");

    fcntl(input_sock, F_SETFL, O_NONBLOCK);
    gettimeofday(&start, NULL);'''
)

old = '''int DG_GetKey(int *pressed, unsigned char *key) {
    return 0;
}'''

new = '''int DG_GetKey(int *pressed, unsigned char *key) {
    unsigned char msg[2];
    int n = recv(input_sock, msg, 2, MSG_DONTWAIT);
    if (n != 2) return 0;
    *pressed = msg[0];
    *key = msg[1];
    return 1;
}'''

if old not in s:
    raise SystemExit("DG_GetKey block not found; source unchanged")

s = s.replace(old, new)
open(p, 'w').write(s)
print("UDP input patch applied.")
