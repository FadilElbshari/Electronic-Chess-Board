import serial
import sys
import time

ACK = b'\x06'  # Arduino sends this when ready for next line
NAK = b'\x15'  # Arduino sends this on error
TIMEOUT = 5    # seconds to wait for response

file_name = sys.argv[1]
port = sys.argv[2]
baud_rate = int(sys.argv[3])

# open serial
ser = serial.Serial(port, baud_rate, timeout=TIMEOUT)
time.sleep(2)  # give Arduino time to reset

count = 1

with open(file_name, 'r') as file:
    for line in file:
        line = line.strip()
        if not line or not line.startswith(':'):
            continue

        # send the line to Arduino
        ser.write(line.encode() + b'\n')
        print(f"{count}. Sent: {line}", end="  ")
        count += 1

        # wait for Arduino to confirm
        resp = ser.read(1)
        if resp == ACK:
            print("✅ ACK")
            continue
        elif resp == NAK:
            print("❌ NAK (Arduino error)")
            # optionally retry
            continue
        else:
            print("⚠️ No response (timeout)")
            break

# tell Arduino we're done (optional)
ser.write(b':END\n')
ser.close()

print("✅ Flashing completed (or stopped).")
