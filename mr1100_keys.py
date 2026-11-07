import ctypes, socket, sys, time
ip = sys.argv[1]
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
keys = {0x57:0xAD, 0x53:0xAF, 0x41:0xAC, 0x44:0xAE, 0x20:0xA2, 0x11:0xA3, 0x1B:27, 0x0D:13}
held = set()
print(f"Sending keys to {ip}:23232")
try:
    while True:
        for vk, key in keys.items():
            down = bool(ctypes.windll.user32.GetAsyncKeyState(vk) & 0x8000)
            if down and vk not in held:
                sock.sendto(bytes([1,key]), (ip,23232))
                held.add(vk)
            elif not down and vk in held:
                sock.sendto(bytes([0,key]), (ip,23232))
                held.remove(vk)
        time.sleep(0.008)
except KeyboardInterrupt:
    for vk in held:
        sock.sendto(bytes([0,keys[vk]]), (ip,23232))
