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
    with open(f"{"alsr" if caches else "noaslr"}-{time.time()}.csv", "w") as f:
        f.write("time(cycles)\n")
        print(res)
        for i in range(1, len(res)):
            if (i + 1) >= len(res): 
                break
            if i % 2 == 0:
                f.write(f"{int(res[i + 1]) - int(res[i])}\n")


if __name__ == "__main__":
    started: bool = False
    done: bool = False
    res: List[str] = list()
    setup_done : bool = False

    global caches
    caches: bool = False
    type: str = ""

    ser = serial.Serial(PORT, baudrate=BAUDRATE)  # open serial port
    ser.write(CMD)     # write a string
    idx: int = 0
    while not done:
        line = do_read(ser, 1)
        print(line)
        if started and not setup_done:
            caches = line == "y"
            setup_done = True
        elif started :
            idx += 1
            res.append(line)
            if idx == 100:
                break
            continue
        if line == "===START===":
            started = True
    print(f"has alsr : {caches}")
    write_csv(res)

    ser.close()             # close port
