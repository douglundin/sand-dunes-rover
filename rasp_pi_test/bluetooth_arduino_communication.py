import serial
import time

PORT = '/dev/tty.HC-06'
BAUD_RATE = 9600

ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
ser.flush()
print("Connected")
time.sleep(2)

ser.write(b"test\n")
print("Sent: test")
time.sleep(1)
response = ser.read_all().decode(errors='replace')
print(f"Received: {response}")
ser.close()