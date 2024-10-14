from typing import List
import serial
import time

BAUDRATE = 1500000
PORT = '/dev/ttyUSB0'
CMD =b"""
mmc read $pxefile_addr_r 0x100000 0x11b\n
bootm start $pxefile_addr_r\n
bootm loados\n
bootm go\n
"""


def do_read(serial: serial.Serial, timeout) -> str:
    buffer: List[bytes] = list()
    before: float  = time.time()

    while time.time() - before < timeout :
        if not serial.readable():
            break
        buffer.append(serial.read(1))
        if buffer[-1] == b'\n':
            break

    return b''.join(buffer).decode('utf-8').removesuffix("\n").removesuffix("\r")

def write_csv(res) -> None:
    with open(f"{"caches" if caches else "nocaches"}-{"alsr" if aslr else "noaslr"}-{time.time()}.csv", "w") as f:
        f.write("time(cycles)\n")
        for i in range(1, len(res) - 2, 2):
            f.write(f"{int(res[i + 1]) - int(res[i])}\n")


if __name__ == "__main__":
    res: List[str] = list()
    started: int = -1
    idx: int = 0

    global caches
    global aslr
    caches: bool = False
    aslr: bool = False

    ser = serial.Serial(PORT, baudrate=BAUDRATE)  # open serial port
    ser.write(CMD)     # write a string

    while True:
        line = do_read(ser, 1)
        print(line)
        if 0 <= started < 2:
            if started == 0:
                aslr = line == "y"          
            else:
                caches = line == "y"
            started += 1
        elif started >= 2:
            idx += 1
            res.append(line)
            if idx == 100:
                break
            continue
        if line == "===START===":
            started = 0
    print(f"has alsr : {caches}")
    write_csv(res)

    ser.close()             # close port
