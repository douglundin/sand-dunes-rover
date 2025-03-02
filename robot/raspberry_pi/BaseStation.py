import socket
import threading
from dataclasses import dataclass
import time
from shared.Communication import Communication, LidarData, CameraFrame, StatusData
from typing import Dict, Any
import numpy as np

@dataclass
class BaseStationConfig:
    base_host: str = '127.0.0.1'
    base_port: int = 2025

class BaseStation(threading.Thread):
    def __init__(self, config: BaseStationConfig):
        # Call the parent class's __init__ method
        super().__init__(daemon=True)
        
        # networking
        self.base_host = config.base_host
        self.base_port = config.base_port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # threading
        self._stop_event = threading.Event()
        
        # Store messages to send to base station
        self.camera_frame = None
        self.sensor_data = None
        self.status_data = None
        self.last_send_time = 0
        self.send_interval = 0.05  # 50ms between sends (20fps max)
    
    def connect_to_base(self) -> bool:
        """Connect to base station"""
        # Add a timeout to the socket connection (e.g., 5 seconds)
        self.socket.settimeout(5)
        try:
            self.socket.connect((self.base_host, self.base_port))
            return True
        except socket.timeout:
          print(f"Connection timed out to {self.base_host}:{self.base_port}")
          return False
        except socket.error as e:
            print(f"Connection failed: {e}")
            return False
    
    def set_camera_frame(self, frame: CameraFrame) -> None:
        """Update the camera frame to be sent"""
        self.camera_frame = CameraFrame(frame=frame)
    
    def set_sensor_data(self, readings: Dict[str, float]) -> None:
        """Update the sensor data to be sent"""
        self.sensor_data = LidarData(readings=readings)
    
    def set_status(self, status: Dict[str, Any]) -> None:
        """Update the robot status to be sent"""
        self.status_data = StatusData(status=status)
    
    def run(self):
        """Main thread function"""
        try:
            if not self.connect_to_base():
                return
            
            while not self.stopped():
                current_time = time.time()
                elapsed_time = current_time - self.last_send_time
                # Rate limiting
                if elapsed_time < self.send_interval:
                    time.sleep(0.01)  # Small sleep to prevent CPU spinning
                    continue
                
                # Send data that's available
                if self.camera_frame:
                    Communication.send(self.socket, self.camera_frame)
                
                if self.sensor_data:
                    Communication.send(self.socket, self.sensor_data)
                
                if self.status_data:
                    Communication.send(self.socket, self.status_data)
                
                self.last_send_time = current_time
                
                # If nothing to send, sleep longer
                if not any([self.camera_frame, self.sensor_data, self.status_data]):
                    time.sleep(1.0)
                    
        except Exception as e:
            print(f"BaseStation error: {e}")
        finally:
            print("Base Station stream ending cleanly...")
            self.close()
    
    def stop(self) -> None:
        """Set stop event"""
        self._stop_event.set()
    
    def stopped(self) -> bool:
        """Check if stopped"""
        return self._stop_event.is_set()
    
    def close(self) -> None:
        """Clean shutdown"""
        self.stop()
        try:
            self.socket.close()
        except:
            pass
        self.join(timeout=2)