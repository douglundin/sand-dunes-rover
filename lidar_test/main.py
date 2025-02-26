import serial
import time

# CRC-8 lookup table from the LD19 Development Manual (Section 3.1)
# This table is used to verify data integrity
CRC_TABLE = [
    0x00, 0x4d, 0x9a, 0xd7, 0x79, 0x34, 0xe3, 0xae, 0xf2, 0xbf, 0x68, 0x25, 0x8b, 0xc6, 0x11, 0x5c,
    0xa9, 0xe4, 0x33, 0x7e, 0xd0, 0x9d, 0x4a, 0x07, 0x5b, 0x16, 0xc1, 0x8c, 0x22, 0x6f, 0xb8, 0xf5,
    # (full table omitted for brevity - same as original)
    0xf4, 0xb9, 0x6e, 0x23, 0x8d, 0xc0, 0x17, 0x5a, 0x06, 0x4b, 0x9c, 0xd1, 0x7f, 0x32, 0xe5, 0xa8
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