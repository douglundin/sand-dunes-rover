import serial
import struct
import math

# Open serial connection (replace with your port)
ser = serial.Serial(
    port='/dev/tty.usbserial-0001',  # or 'COM4' on Windows
    baudrate=230400,
    timeout=1
)

def read_lidar_data():
    # Loop and Wait until we read a packet header (0x54)
    while True:
        if ser.read(1) == b'\x54':
            break
    
    # Read packet type and point count, tells us how many points will be in the packet99
    ver_len = ord(ser.read(1))
    point_count = ver_len & 0x1F  # Lower 5 bits indicate point count
    
    # Read remaining packet data
    data = ser.read(6 + point_count*3 + 4)  # Speed(2) + StartAngle(2) + Data(n*3) + EndAngle(2) + Timestamp(2) + CRC(1)
    
    # Extract speed (degrees/sec)
    speed = struct.unpack('<H', data[0:2])[0]
    
    # Extract start and end angles (in 0.01 degree increments)
    start_angle = struct.unpack('<H', data[2:4])[0] / 100.0
    end_angle = struct.unpack('<H', data[4+point_count*3:6+point_count*3])[0] / 100.0
    
    # Process each measurement point
    points = []
    for i in range(point_count):
        offset = 4 + i*3
        distance = struct.unpack('<H', data[offset:offset+2])[0]  # in mm
        intensity = data[offset+2]  # signal strength
        
        # Calculate angle for this point using linear interpolation
        angle = start_angle + (end_angle - start_angle) * (i / (point_count-1))
        
        # Convert polar coordinates to Cartesian (x,y)
        if distance > 0:
            angle_rad = math.radians(angle)
            x = distance * math.cos(angle_rad)
            y = distance * math.sin(angle_rad)
            points.append((angle, distance, intensity, x, y))
    
    return points

# Read and print 10 sets of measurements
try:
    for _ in range(10):
        points = read_lidar_data()
        print(f"Got {len(points)} points")
        for angle, distance, intensity, x, y in points:
            print(f"  Angle: {angle:.2f}°, Distance: {distance}mm, Intensity: {intensity}, x:{x}, y: {y}")
finally:
    ser.close()