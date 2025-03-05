import serial
import struct
import math
import time

# Open serial connection (replace with your port)
ser = serial.Serial(
    port='/dev/ttyUSB0',  # or 'COM4' on Windows
    baudrate=230400,
    timeout=1
)

# FieldSize (Bytes)Description
# Header        1 Byte     Always 0x54 (start of packet)
# VerLen        1 Byte     Upper 3 bits: Packet type Lower 5 bits: Point count
# Speed         2 Bytes    Rotation speed (degrees/sec)
# Start Angle   2 Bytes    Start angle (0.01° increments)
# Data Points   3 * N Bytes Each point: Distance (2 Bytes) + Intensity (1 Byte)
# End Angle     2 Bytes    End angle (0.01° increments)
# Timestamp     2 Bytes    Timestamp (ms)
# CRC Check     1 Byte     Packet validation

def read_lidar_data():
    # Loop and Wait until we read a packet header (0x54)
    while True:
        if ser.read(1) == b'\x54':
            break
    
    # Read packet type and point count, tells us how many points will be in the packet
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
        
        # Convert polar coordinates to Cartesian (x,y) in mm
        if distance > 0:
            angle_rad = math.radians(angle)
            x = distance * math.cos(angle_rad)
            y = distance * math.sin(angle_rad)
            points.append((angle, distance, intensity, x, y))
    
    return points # [ (angle,  distance, intensity, x, y), (angle,  distance, intensity, x, y), ...]

# Read and print 10 sets of measurements
try:
    while True:
      points = read_lidar_data()
      print(f"Got {len(points)} points")
      for angle, distance, intensity, x, y in points:
          print(f"  Angle: {angle:.2f}°, Distance: {distance}mm, Intensity: {intensity}\nx:{x:.1f}mm, y: {y:1f}mm\n\n\n")
      time.sleep(2)
finally:
    ser.close()