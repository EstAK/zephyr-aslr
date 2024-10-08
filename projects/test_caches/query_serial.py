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
    with open(f"{"caches" if caches else "nocaches"}-{time.time()}.csv", "w") as f:
        f.write("size, time(cycles)\n")
        for elem in res:
            f.write(f"{elem}\n")


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
    while not done:
        line = do_read(ser, 1)
        if started and not setup_done:
            print(line)
            caches = line == "y"
            setup_done = True
        elif started :
            if line == "===END===":
                done = True
                break
            res.append(line)
            print(line)
            continue
        if line == "===START===":
            started = True
    print(f"has caches : {caches}")
    write_csv(res)

    ser.close()             # close port
