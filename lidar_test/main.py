import serial
import time

# CRC-8 lookup table from the manual (Section 3.1)
CRC_TABLE = [
    0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c,
    0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5,
    # ... (full table omitted for brevity, copy from Development Manual page 8-9)
    0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
]

def calculate_crc(data):
    crc = 0
    for byte in data:
        crc = CRC_TABLE[(crc ^ byte) & 0xff]
    return crc

def parse_packet(data):
    if len(data) != 47 or data[0] != 0x54:  # Check packet length and header
        return None
    
    # Extract fields (little-endian byte order)
    speed = int.from_bytes(data[2:4], 'little')  # Degrees per second
    start_angle = int.from_bytes(data[4:6], 'little') / 100.0  # Degrees
    end_angle = int.from_bytes(data[42:44], 'little') / 100.0  # Degrees
    timestamp = int.from_bytes(data[44:46], 'little')  # Milliseconds
    crc = data[46]
    
    # Verify CRC
    calculated_crc = calculate_crc(data[:-1])
    if crc != calculated_crc:
        return None
    
    # Parse 12 measurement points (3 bytes each: 2 for distance, 1 for intensity)
    points = []
    for i in range(12):
        offset = 6 + i * 3
        distance = int.from_bytes(data[offset:offset+2], 'little')  # mm
        intensity = data[offset+2]
        # Calculate angle by interpolation
        angle_step = (end_angle - start_angle) / 11  # 11 steps for 12 points
        angle = start_angle + i * angle_step
        points.append((angle, distance, intensity))
    
    return speed, start_angle, end_angle, timestamp, points

def main():
    # Replace with your serial port (e.g., 'COM4' on Windows, '/dev/ttyUSB0' on Linux)
    port = '/dev/tty.usbserial-0001'  # Example for Linux
    baud_rate = 230400

    try:
        # Open serial connection
        ser = serial.Serial(port, baud_rate, timeout=1)
        print(f"Connected to {port} at {baud_rate} baud")
        
        while True:
            # Read 47 bytes (one packet)
            data = ser.read(47)
            if len(data) == 47:
                result = parse_packet(data)
                if result:
                    speed, start_angle, end_angle, timestamp, points = result
                    print(f"Speed: {speed} deg/s, Start Angle: {start_angle:.2f}°, "
                          f"End Angle: {end_angle:.2f}°, Timestamp: {timestamp} ms")
                    for i, (angle, dist, intens) in enumerate(points):
                        print(f"Point {i+1}: Angle={angle:.2f}°, Distance={dist} mm, Intensity={intens}")
                    print("-" * 50)
            time.sleep(0.01)  # Small delay to avoid overwhelming the CPU
    except serial.SerialException as e:
        print(f"Error: Could not open serial port - {e}")
    except KeyboardInterrupt:
        print("Stopped by user")
    finally:
        if 'ser' in locals():
            ser.close()
            print("Serial port closed")

if __name__ == "__main__":
    main()