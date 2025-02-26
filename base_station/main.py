import socket
import pickle
import struct
import cv2
import threading
import queue
import time
from typing import Optional, Dict, Any
import sys
import os

# Get the project root (one level up from base_station)
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

# Import shared components
from shared.Communication import CameraFrame, LidarData, MessageType, Message

class DataBuffer:
    """Thread-safe buffer with size limit for any type of data"""
    def __init__(self, maxsize: int = 90):
        self.items = queue.Queue(maxsize=maxsize)
        
    def put(self, item: Any) -> bool:
        """Add item to buffer, dropping oldest if full. Returns True if successful."""
        try:
            self.items.put_nowait(item)
            return True
        except queue.Full:
            try:
                self.items.get_nowait()  # Drop oldest
                self.items.put_nowait(item)
                return True
            except (queue.Empty, queue.Full):
                return False

    def get(self) -> Optional[Any]:
        """Get next item from buffer"""
        try:
            return self.items.get_nowait()
        except queue.Empty:
            return None
            
    def get_blocking(self, timeout: float = 1.0) -> Optional[Any]:
        """Get next item from buffer, blocking until timeout"""
        try:
            return self.items.get(timeout=timeout)
        except queue.Empty:
            return None
            
    def empty(self) -> bool:
        """Check if buffer is empty"""
        return self.items.empty()
        
    def qsize(self) -> int:
        """Get number of items in buffer"""
        return self.items.qsize()

class BaseStation(threading.Thread):
    def __init__(self, host: str = "0.0.0.0", port: int = 2025):
        super().__init__(daemon=True)
        # Network configuration
        self.host = host
        self.port = port
        self.server_socket = None
        self.client_socket = None
        self.client_address = None
        
        # Threading and state
        self.is_running = False
        self._stop_event = threading.Event()
        
        # Data buffers for different message types
        self.frame_buffer = DataBuffer(maxsize=90)
        self.lidar_buffer = DataBuffer(maxsize=30)
        self.telemetry_buffer = DataBuffer(maxsize=100)
        
        # Receive buffer for raw socket data
        self.data_buffer = b""
        self.payload_size = struct.calcsize("L")  # unsigned long for message size
        
        # Display configuration
        self.display_enabled = True
        self.display_lidar = False
        
        # Stats tracking
        self.stats = {
            "frames_received": 0,
            "lidar_received": 0,
            "telemetry_received": 0,
            "bytes_received": 0,
            "fps": 0,
            "start_time": 0,
            "last_update": 0
        }

    def setup_server(self) -> bool:
        """Initialize server socket"""
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # Allow address reuse
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(1)
            print(f"Base station started on {self.host}:{self.port}, waiting for robot connection...")
            return True
        except Exception as e:
            print(f"Base station setup failed: {e}")
            self.stop()
            return False

    def accept_connection(self) -> bool:
        """Accept robot connection"""
        try:
            self.client_socket, self.client_address = self.server_socket.accept()
            print(f"Connected to robot at {self.client_address}")
            self.stats["start_time"] = time.time()
            self.stats["last_update"] = time.time()
            return True
        except Exception as e:
            print(f"Connection acceptance failed: {e}")
            return False

    def receive_message_size(self) -> Optional[int]:
        """Get size of next message from the robot"""
        try:
            while len(self.data_buffer) < self.payload_size:
                packet = self.client_socket.recv(4096)
                if not packet:
                    print("Connection to robot lost")
                    return None
                self.data_buffer += packet
                self.stats["bytes_received"] += len(packet)

            packed_msg_size = self.data_buffer[:self.payload_size]
            self.data_buffer = self.data_buffer[self.payload_size:]
            return struct.unpack("L", packed_msg_size)[0]
        except Exception as e:
            print(f"Error receiving message size: {e}")
            return None

    def receive_message_data(self, msg_size: int) -> Optional[bytes]:
        """Get message data of specified size"""
        try:
            while len(self.data_buffer) < msg_size:
                packet = self.client_socket.recv(4096)
                if not packet:
                    print("Connection to robot lost during data transfer")
                    return None
                self.data_buffer += packet
                self.stats["bytes_received"] += len(packet)

            message_data = self.data_buffer[:msg_size]
            self.data_buffer = self.data_buffer[msg_size:]
            return message_data
        except Exception as e:
            print(f"Error receiving message data: {e}")
            return None

    def process_message(self, message_data: bytes) -> bool:
        """Process received message based on type and add to appropriate buffer"""
        try:
            # Try to determine message type (this might vary depending on serialization)
            # If using Communication.receive, we could use that instead
            message = pickle.loads(message_data)
            
            # Sort by message type
            if isinstance(message, CameraFrame):
                self.frame_buffer.put(message)
                self.stats["frames_received"] += 1
            elif isinstance(message, LidarData):
                self.lidar_buffer.put(message)
                self.stats["lidar_received"] += 1
            elif message.message_type == MessageType.TELEMETRY:
                self.telemetry_buffer.put(message)
                self.stats["telemetry_received"] += 1
            else:
                print(f"Unknown message type: {message.message_type}")
                
            # Update statistics
            current_time = time.time()
            if current_time - self.stats["last_update"] >= 1.0:
                elapsed = current_time - self.stats["start_time"]
                if elapsed > 0:
                    self.stats["fps"] = self.stats["frames_received"] / elapsed
                self.stats["last_update"] = current_time
                
            return True
        except Exception as e:
            print(f"Error processing message: {e}")
            return False

    def display_camera_feed(self, window_name: str = 'Robot Camera Feed') -> None:
        """Display camera frames with annotations"""
        frame_obj = self.frame_buffer.get()
        if frame_obj is not None:
            # Get the image from the CameraFrame
            if hasattr(frame_obj, 'frame') and frame_obj.frame is not None:
                display_image = frame_obj.frame.copy()
            elif hasattr(frame_obj, 'image') and frame_obj.image is not None:
                display_image = frame_obj.image.copy()
            else:
                return  # No valid image data
            
            # Add status information to the frame
            font = cv2.FONT_HERSHEY_SIMPLEX
            cv2.putText(display_image, f"FPS: {self.stats['fps']:.1f}", (10, 30), font, 1, (0, 255, 0), 2)
            cv2.putText(display_image, f"Frames: {self.stats['frames_received']}", (10, 70), font, 1, (0, 255, 0), 2)
            
            # Add additional metadata if available
            if hasattr(frame_obj, 'metadata') and frame_obj.metadata:
                y_pos = 110
                for key, value in frame_obj.metadata.items():
                    if key not in ['timestamp']:  # Skip some metadata
                        cv2.putText(display_image, f"{key}: {value}", (10, y_pos), 
                                   font, 0.8, (0, 255, 0), 2)
                        y_pos += 40
            
            # Display the frame
            cv2.imshow(window_name, display_image)

    def display_lidar_data(self, window_name: str = 'Robot Lidar Data') -> None:
        """Display lidar data as a visualization"""
        lidar_data = self.lidar_buffer.get()
        if lidar_data is not None and hasattr(lidar_data, 'readings'):
            # Create a simple visualization (this will depend on your lidar data format)
            # For demonstration, we'll create a simple top-down view
            width, height = 800, 800
            center_x, center_y = width // 2, height // 2
            max_range = 300  # Max pixel distance from center
            
            # Create blank image
            lidar_viz = np.zeros((height, width, 3), dtype=np.uint8)
            
            # Draw robot position
            cv2.circle(lidar_viz, (center_x, center_y), 10, (0, 0, 255), -1)
            
            # Draw readings
            for angle, distance in lidar_data.readings.items():
                # Convert angle to radians
                angle_rad = float(angle) * 3.14159 / 180.0
                
                # Calculate endpoint
                scale_factor = min(1.0, max_range / distance) if distance > 0 else 0
                x = int(center_x + np.cos(angle_rad) * distance * scale_factor)
                y = int(center_y + np.sin(angle_rad) * distance * scale_factor)
                
                # Draw line from center to endpoint
                cv2.line(lidar_viz, (center_x, center_y), (x, y), (0, 255, 0), 1)
                
            # Display information
            font = cv2.FONT_HERSHEY_SIMPLEX
            cv2.putText(lidar_viz, f"Lidar Points: {len(lidar_data.readings)}", 
                       (10, 30), font, 1, (255, 255, 255), 2)
            
            # Show visualization
            cv2.imshow(window_name, lidar_viz)

    def display_telemetry(self, window_name: str = 'Robot Telemetry') -> None:
        """Display telemetry data"""
        # Implementation will depend on your telemetry format
        # This is a placeholder for custom visualization
        pass

    def connect(self) -> bool:
        """Start server and wait for connection"""
        try:
            if not self.setup_server():
                return False
            
            if not self.accept_connection():
                return False
                
            self.is_running = True
            return True
        except Exception as e:
            print(f"Connection error: {e}")
            return False

    def run(self) -> None:
        """Main processing loop"""
        try:
            if not self.connect():
                return

            while self.is_running and not self._stop_event.is_set():
                msg_size = self.receive_message_size()
                if msg_size is None:
                    break

                message_data = self.receive_message_data(msg_size)
                if message_data is None:
                    break

                if not self.process_message(message_data):
                    break

        except Exception as e:
            print(f"Message processing error: {e}")
        finally:
            self.stop()

    def stop(self) -> None:
        """Clean shutdown"""
        self._stop_event.set()
        self.is_running = False
        
        if self.client_socket:
            try:
                self.client_socket.close()
            except Exception as e:
                print(f"Error closing client socket: {e}")
            self.client_socket = None

        if self.server_socket:
            try:
                self.server_socket.close()
            except Exception as e:
                print(f"Error closing server socket: {e}")
            self.server_socket = None

        cv2.destroyAllWindows()

    def toggle_display(self, enable: bool = None) -> None:
        """Toggle whether to display camera feed"""
        if enable is not None:
            self.display_enabled = enable
        else:
            self.display_enabled = not self.display_enabled

    def toggle_lidar_display(self, enable: bool = None) -> None:
        """Toggle whether to display lidar data"""
        if enable is not None:
            self.display_lidar = enable
        else:
            self.display_lidar = not self.display_lidar

def main():
    # Import numpy inside the function to ensure proper init
    import numpy as np
    
    base_station = BaseStation("0.0.0.0", 2025)

    try:
        base_station.start()
        print("Base station started, processing data...")
        
        # Main display loop
        while not base_station._stop_event.is_set():
            if base_station.display_enabled:
                base_station.display_camera_feed()
                
            if base_station.display_lidar:
                base_station.display_lidar_data()
                
            # Check for keyboard commands
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                print("Quit requested by user")
                break
            elif key == ord('l'):
                base_station.toggle_lidar_display()
                print(f"Lidar display {'enabled' if base_station.display_lidar else 'disabled'}")
            elif key == ord('c'):
                base_station.toggle_display()
                print(f"Camera display {'enabled' if base_station.display_enabled else 'disabled'}")
                
            # Small sleep to prevent CPU spinning
            time.sleep(0.005)
    except KeyboardInterrupt:
        print("Base station stopped by user")
    finally:
        base_station.stop()
        print("Base station shutdown complete")

if __name__ == "__main__":
    main()