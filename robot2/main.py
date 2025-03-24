import serial
import time
import logging #20250324 DB: Added import for logging

#Constants: adding constants to variables rather than hard coding them in #20250324 DB
SLEEP_TIME = 0.01
UART_COMMAND_CLOSED = "UART connection closed"
PORT = '/dev/ttyUSB0'
BAUDRATE = 9600
TIMEOUT = 1
FILE_NAME = 'LOG_FILE'

        #20250324 DB: Implement logging to better track errors. used formating to display time, level, and message in a log file
logging.basicConfig(filename= FILE_NAME, format='%(asctime)s - %(levelname)s - %(message)s', level=logging.INFO)

# Pi connects to arduino controller via usb port

# Configuring the serial port
# '/dev/ttyACM0' is the port on the rasp pi 4 when connecting arduino uno r3 usb port

#20250324 DB: Added a try catch with logging to the serial connection to handle errors when opening the connection
try:
    ser = serial.Serial(
        port= PORT,
        baudrate=BAUDRATE,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=TIMEOUT
)
#20250324 DB: Catch handles error with connection exiting rather than potentially crashing the program
#logs error into the log file before exiting
except serial.SerialException as e:
        logging.error(f"Error opening serial port: {e}")
        print(f"Failed to open serial port: {PORT}")
        exit(1)

print("UART Controller initialized")

try:
    print("\nArduino Robot Controller")
    print("-----------------------")
    print("Commands:")
    print("  w - Forward")
    print("  s - Backward")
    print("  a - Left")
    print("  d - Right")
    print("  x - Stop")
    print("  e - Brake")
    print("  q - Quit")                         #20250324 DB: It appears the input method is manual rather than
                                                # autonomous as stated in documentation

                                                #20250324 DB: Change commands to accommodate WASD method assuming input
                                                # is coming from a QWERTY keyboard for ease of user experience
    while True:
        cmd = input("\nEnter command: ").lower()

        if cmd == 'q':
            break

        if cmd in ['w', 's', 'a', 'd', 'x', 'e']:  #20250324 DB: Added the new characters in here
            ser.write(cmd.encode())  # Send the character as bytes
            print(f"Sent command: {cmd}")
            logging.info(f"Sent command: {cmd}") #20250324 DB: Log command sent
            time.sleep(SLEEP_TIME)  # Small delay between commands
        else:
                print("Invalid command")
                logging.warning(f"Invalid Command entered: {cmd}") #20250324 DB: logging invalid commands

except KeyboardInterrupt:
       logging.info("\nProgram terminated by user") #20250324 DB: Logging user termination

finally:
        ser.close()  # Close the serial port
        print(UART_COMMAND_CLOSED)
        logging.info(UART_COMMAND_CLOSED)  #20250324 DB: Logging serial port closure

