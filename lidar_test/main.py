import serial
import time

# CRC-8 lookup table from the LD19 Development Manual (Section 3.1)
# This table is used to verify data integrity
CRC_TABLE = [
    0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3,
    0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c, 0xa9, 0xe4, 0x33,
    0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8,
    0xf5, 0x1f, 0x52, 0x85, 0xc8, 0x66, 0x2b, 0xfc, 0xb1, 0xed, 0xa0, 0x77,
    0x3a, 0x94, 0xd9, 0x0e, 0x43, 0xb6, 0xfb, 0x2c, 0x61, 0xcf, 0x82, 0x55,
    0x18, 0x44, 0x09, 0xde, 0x93, 0x3d, 0x70, 0xa7, 0xea, 0x3e, 0x73, 0xa4,
    0xe9, 0x47, 0x0a, 0xdd, 0x90, 0xcc, 0x81, 0x56, 0x1b, 0xb5, 0xf8, 0x2f,
    0x62, 0x97, 0xda, 0x0d, 0x40, 0xee, 0xa3, 0x74, 0x39, 0x65, 0x28, 0xff,
    0xb2, 0x1c, 0x51, 0x86, 0xcb, 0x21, 0x6c, 0xbb, 0xf6, 0x58, 0x15, 0xc2,
    0x8f, 0xd3, 0x9e, 0x49, 0x04, 0xaa, 0xe7, 0x30, 0x7d, 0x88, 0xc5, 0x12,
    0x5f, 0xf1, 0xbc, 0x6b, 0x26, 0x7a, 0x37, 0xe0, 0xad, 0x03, 0x4e, 0x99,
    0xd4, 0x7c, 0x31, 0xe6, 0xab, 0x05, 0x48, 0x9f, 0xd2, 0x8e, 0xc3, 0x14,
    0x59, 0xf7, 0xba, 0x6d, 0x20, 0xd5, 0x98, 0x4f, 0x02, 0xac, 0xe1, 0x36,
    0x7b, 0x27, 0x6a, 0xbd, 0xf0, 0x5e, 0x13, 0xc4, 0x89, 0x63, 0x2e, 0xf9,
    0xb4, 0x1a, 0x57, 0x80, 0xcd, 0x91, 0xdc, 0x0b, 0x46, 0xe8, 0xa5, 0x72,
    0x3f, 0xca, 0x87, 0x50, 0x1d, 0xb3, 0xfe, 0x29, 0x64, 0x38, 0x75, 0xa2,
    0xef, 0x41, 0x0c, 0xdb, 0x96, 0x42, 0x0f, 0xd8, 0x95, 0x3b, 0x76, 0xa1,
    0xec, 0xb0, 0xfd, 0x2a, 0x67, 0xc9, 0x84, 0x53, 0x1e, 0xeb, 0xa6, 0x71,
    0x3c, 0x92, 0xdf, 0x08, 0x45, 0x19, 0x54, 0x83, 0xce, 0x60, 0x2d, 0xfa,
    0xb7, 0x5d, 0x10, 0xc7, 0x8a, 0x24, 0x69, 0xbe, 0xf3, 0xaf, 0xe2, 0x35,
    0x78, 0xd6, 0x9b, 0x4c, 0x01, 0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17,
    0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
]

def calculate_crc(data):
    """
    Calculate CRC-8 checksum for validating packet integrity.
    
    Args:
        data: Bytes to calculate CRC on
        
    Returns:
        Calculated CRC-8 value
    """
    crc = 0
    for byte in data:
        crc = CRC_TABLE[(crc ^ byte) & 0xff]
    return crc

def parse_packet(data):
    """
    Parse a 47-byte packet from the LD19 LIDAR sensor.
    
    Packet structure:
    - Byte 0: Header (0x54)
    - Byte 1: Version and length info (0x2C)
    - Bytes 2-3: Speed (degrees per second)
    - Bytes 4-5: Start angle (0.01 degree units)
    - Bytes 6-41: 12 measurement points (3 bytes each)
    - Bytes 42-43: End angle (0.01 degree units)
    - Bytes 44-45: Timestamp (milliseconds)
    - Byte 46: CRC checksum
    
    Args:
        data: 47-byte packet from LIDAR
        
    Returns:
        Tuple of (speed, start_angle, end_angle, timestamp, points) if valid
        None if packet is invalid
    """
    # Check packet length and header signature
    if len(data) != 47 or data[0] != 0x54:
        return None
    
    # Extract metadata fields (using little-endian byte order)
    speed = int.from_bytes(data[2:4], 'little')           # Rotation speed in degrees per second
    start_angle = int.from_bytes(data[4:6], 'little') / 100.0  # Start angle in degrees
    end_angle = int.from_bytes(data[42:44], 'little') / 100.0  # End angle in degrees
    timestamp = int.from_bytes(data[44:46], 'little')     # Timestamp in milliseconds
    crc = data[46]                                         # CRC checksum byte
    
    # Verify packet integrity with CRC
    calculated_crc = calculate_crc(data[:-1])
    if crc != calculated_crc:
        print(f"CRC error: Got {crc}, calculated {calculated_crc}")
        return None
    
    # Parse the 12 measurement points (each point is 3 bytes)
    points = []
    for i in range(12):
        # Calculate the offset within the packet for this point
        offset = 6 + i * 3
        
        # First 2 bytes are distance in mm, 3rd byte is intensity
        distance = int.from_bytes(data[offset:offset+2], 'little')  # Distance in mm
        intensity = data[offset+2]                                 # Signal intensity
        
        # Calculate the angle of this point by linear interpolation between start and end angles
        angle_step = (end_angle - start_angle) / 11  # 11 steps for 12 points
        angle = start_angle + i * angle_step
        
        # Store the point data as a tuple of (angle, distance, intensity)
        points.append((angle, distance, intensity))
    
    return speed, start_angle, end_angle, timestamp, points

def main():
    """
    Main function to read and process LIDAR data continuously.
    """
    # Serial port configuration - change this to match your system
    port = '/dev/tty.usbserial-0001'  # Example: '/dev/ttyUSB0' on Linux, 'COM4' on Windows
    baud_rate = 230400                # Must match sensor's baud rate (230400 for LD19)
    
    try:
        # Open serial connection to the LIDAR sensor
        print(f"Attempting to connect to {port} at {baud_rate} baud...")
        ser = serial.Serial(port, baud_rate, timeout=1)
        print(f"Connected to {port} at {baud_rate} baud")
        
        # Create lists to store point data for visualization file
        all_angles = []
        all_distances = []
        all_intensities = []
        
        # Main processing loop
        scan_count = 0
        last_time = time.time()
        
        while True:
            # Read one 47-byte packet
            data = ser.read(47)
            
            # Only process if we received a full packet
            if len(data) == 47:
                # Parse the packet
                result = parse_packet(data)
                
                if result:
                    # Unpack the result
                    speed, start_angle, end_angle, timestamp, points = result
                    
                    scan_count += 1
                    
                    # Print summary every 100 packets to avoid console flooding
                    if scan_count % 100 == 0:
                        current_time = time.time()
                        elapsed = current_time - last_time
                        print(f"Processed {scan_count} packets. Rate: {100/elapsed:.2f} packets/second")
                        print(f"Speed: {speed} deg/s, Start Angle: {start_angle:.2f}°, "
                              f"End Angle: {end_angle:.2f}°, Timestamp: {timestamp} ms")
                        last_time = current_time
                    
                    # Store data for optional saving to a file for visualization
                    for angle, dist, intens in points:
                        all_angles.append(angle)
                        all_distances.append(dist)
                        all_intensities.append(intens)
                        
                        # Uncomment to print every point (will flood console)
                        # print(f"Angle={angle:.2f}°, Distance={dist} mm, Intensity={intens}")
                    
                    # Limit data storage to avoid memory issues during long runs
                    max_points = 10000  # Adjust as needed
                    if len(all_angles) > max_points:
                        all_angles = all_angles[-max_points:]
                        all_distances = all_distances[-max_points:]
                        all_intensities = all_intensities[-max_points:]
            
            # Small delay to avoid overwhelming the CPU
            time.sleep(0.01)
            
    except serial.SerialException as e:
        print(f"Error: Could not open serial port - {e}")
    except KeyboardInterrupt:
        print("Stopped by user (Ctrl+C)")
    finally:
        # Make sure to close the serial port when done
        if 'ser' in locals():
            ser.close()
            print("Serial port closed")
        
        # Save collected data to file for visualization
        try:
            if 'all_angles' in locals() and len(all_angles) > 0:
                print(f"Saving {len(all_angles)} data points to lidar_data.csv")
                with open('lidar_data.csv', 'w') as f:
                    f.write("angle,distance,intensity\n")
                    for i in range(len(all_angles)):
                        f.write(f"{all_angles[i]},{all_distances[i]},{all_intensities[i]}\n")
                print("Data saved successfully!")
        except Exception as e:
            print(f"Error saving data: {e}")

if __name__ == "__main__":
    main()