import cv2
import socket
import pickle
import struct
import threading
import time
from shared.shared_types import Frame
import sys
import os

# Get the project root (one level up from base_station)
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from shared.shared_types import Frame

class ThreadLogging:
    @staticmethod
    def get_thread_info():
        print(f"Threads alive count: {threading.active_count()}\n"
              f"Thread current: {threading.current_thread()}")

class CameraStreamer(threading.Thread):
    def __init__(self, host='BASE_STATION_IP', port=2025, 
                 camera_id=0, fps_target=30):
        super().__init__(daemon=True)
        self.host = host
        self.port = port
        self.camera_id = camera_id
        self.fps_target = fps_target
        self.frame_interval = 1.0 / fps_target
        
        # Initialize components
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.camera = None
        self._stop_event = threading.Event()
        self.frame_count = 0
        
        # Performance monitoring
        self.fps_stats = {'count': 0, 'start_time': time.time()}

    def setup_camera(self) -> bool:
        """Initialize the camera with desired settings"""
        self.camera = cv2.VideoCapture(self.camera_id)
        if not self.camera.isOpened():
            print(f"Failed to open camera {self.camera_id}")
            return False
            
        # Set camera properties
        self.camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        self.camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        self.camera.set(cv2.CAP_PROP_FPS, self.fps_target)
        return True

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()

    def connect(self) -> bool:
        """Connect to base station"""
        try:
            self.socket.connect((self.host, self.port))
            return True
        except socket.error as e:
            print(f"Connection failed: {e}")
            return False

    def capture_frame(self) -> Optional[Frame]:
        """Capture and create a Frame object"""
        ret, image = self.camera.read()
        if not ret:
            return None
            
        # Update FPS calculation
        self.fps_stats['count'] += 1
        current_time = time.time()
        elapsed = current_time - self.fps_stats['start_time']
        
        if elapsed > 1.0:
            fps = self.fps_stats['count'] / elapsed
            self.fps_stats = {'count': 0, 'start_time': current_time}
        else:
            fps = self.fps_stats['count'] / elapsed
            
        # Create frame with metadata
        frame = Frame.create(image, self.frame_count)
        frame.metadata['fps'] = fps
        frame.metadata['camera_id'] = self.camera_id
        
        self.frame_count += 1
        return frame

    def send_frame(self, frame: Frame) -> bool:
        """Serialize and send a frame"""
        try:
            data = pickle.dumps(frame)
            message_size = struct.pack("L", len(data))
            self.socket.sendall(message_size + data)
            return True
        except socket.error as e:
            print(f"Failed to send: {e}")
            return False

    def run(self):
        """Main streaming loop"""
        if not self.setup_camera():
            return

        last_frame_time = time.time()
        
        while not self.stopped():
            current_time = time.time()
            
            # Maintain target FPS
            if current_time - last_frame_time < self.frame_interval:
                time.sleep(0.001)  # Short sleep to prevent CPU spinning
                continue
                
            frame = self.capture_frame()
            if frame is None:
                break
                
            if not self.send_frame(frame):
                break
                
            last_frame_time = current_time

        print("Camera stream ending cleanly...")

    def close(self):
        """Clean shutdown"""
        self.stop()
        self.join(timeout=2)
        if self.camera:
            self.camera.release()
        self.socket.close()

def main():
    camera_streamer = CameraStreamer(host='192.168.50.129', port=2025)
    
    try:
        if camera_streamer.connect():
            print("Connected to base station")
            camera_streamer.start()
            
            while True:
                ThreadLogging.get_thread_info()
                time.sleep(0.1)
                
    except KeyboardInterrupt:
        print("Streaming stopped by user")
    finally:
        camera_streamer.close()

if __name__ == "__main__":
    main()