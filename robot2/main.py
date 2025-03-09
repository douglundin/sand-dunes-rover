import serial
import time

# Pi connects to Arduino controller via USB port
# '/dev/ttyACM0' is typical for Arduino Uno R3 on Raspberry Pi 4
# Check port with 'ls /dev/tty*' before and after connecting Arduino
serial_port = '/dev/ttyACM0'
baud_rate = 9600

# Configure the serial port
ser = serial.Serial(
    port=serial_port,
    baudrate=baud_rate,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1  # Timeout in seconds for read operations
)

print("UART Controller initialized")

def parse_sensor_data(data):
    """Parse the comma-separated MPU6050 data string into meaningful values.
    
    Expected format: 'ax,ay,az,gx,gy,gz,temp'
    Returns: Dictionary with sensor values or None if parsing fails
    """
    try:
        # Split the data string by commas
        values = data.split(',')
        if len(values) != 7:  # Expecting 7 values
            return None
        
        # Convert string values to appropriate types
        sensor_data = {
            'accel_x': float(values[0]),  # Accelerometer X (raw)
            'accel_y': float(values[1]),  # Accelerometer Y (raw)
            'accel_z': float(values[2]),  # Accelerometer Z (raw)
            'gyro_x': float(values[3]),   # Gyroscope X (raw)
            'gyro_y': float(values[4]),   # Gyroscope Y (raw)
            'gyro_z': float(values[5]),   # Gyroscope Z (raw)
            'temp': float(values[6])      # Temperature in Celsius
        }
        return sensor_data
    
    except (ValueError, IndexError) as e:
        print(f"Error parsing sensor data: {e}")
        return None

def display_sensor_data(sensor_data):
    """Display the parsed sensor data in a readable format."""
    if sensor_data:
        print("\nMPU6050 Sensor Data:")
        print(f"Accelerometer (raw): X={sensor_data['accel_x']:.0f}, "
              f"Y={sensor_data['accel_y']:.0f}, Z={sensor_data['accel_z']:.0f}")
        print(f"Gyroscope (raw): X={sensor_data['gyro_x']:.0f}, "
              f"Y={sensor_data['gyro_y']:.0f}, Z={sensor_data['gyro_z']:.0f}")
        print(f"Temperature: {sensor_data['temp']:.2f}°C")
    else:
        print("Failed to display sensor data - invalid format")

try:
    print("\nArduino Robot Controller")
    print("-----------------------")
    print("Commands:")
    print("  f - Forward")
    print("  b - Backward")
    print("  l - Left")
    print("  r - Right")
    print("  s - Stop")
    print("  t - Brake")
    print("  q - Quit")
    
    # Give some time for Arduino to initialize
    time.sleep(2)
    
    while True:
        # Check for incoming sensor data
        if ser.in_waiting > 0:
            data = ser.readline().decode('utf-8').strip()  # Read a line and decode
            if data:  # Ensure we got actual data
                sensor_data = parse_sensor_data(data)
                display_sensor_data(sensor_data)
        
        # Handle user input for motor commands
        cmd = input("\nEnter command: ").lower()
        
        if cmd == 'q':
            break
        
        if cmd in ['f', 'b', 'l', 'r', 's', 't']:
            ser.write(cmd.encode())  # Send command as bytes
            print(f"Sent command: {cmd}")
            time.sleep(0.1)  # Small delay to prevent overwhelming Arduino
        else:
            print("Invalid command")

except serial.SerialException as e:
    print(f"Serial error: {e}")
except KeyboardInterrupt:
    print("\nProgram terminated by user")
finally:
    ser.close()  # Ensure the serial port is closed
    print("UART connection closed")