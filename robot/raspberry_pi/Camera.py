import cv2
import threading
import time
from dataclasses import dataclass
from typing import Tuple, Optional
from shared.Communication import CameraFrame
from Buffer import DoubleBuffer
import numpy as np
from shared.Communication import CameraFrame

@dataclass
class CameraConfig():
    buffer: DoubleBuffer
    max_queue_size: int = 40
    fps_target: int = 30

class Camera(threading.Thread): # inherits threading for unblocked access to camera sensor reading
    def __init__(self, config: CameraConfig, camera_id=0):
        # threading setup
        super().__init__(daemon=True) # run this thread as a daemon
        self._stop_camera = threading.Event()

        # sensor
        self.camera_id = camera_id
        self.camera = None

        # sensor data
        self.buffer = config.buffer

        # performance
        self.fps_target = config.fps_target
        self.fps_frame_interval = 1 / self.fps_target # provides seconds per frame (1/frames per second = seconds per frame)

        # monitoring
        self.frame_count = 0 # total frames processed during lifetime of camera thread
        self.fps = None
        self.fps_interval_state = {
            'count': 0, 
            'start_time': time.time()
            }

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

    def stop_camera(self) -> bool:
        self._stop_camera.set()

    def stopped(self) -> bool:
        return self._stop_camera.is_set()

    def capture_sensor(self) -> Optional[np.ndarray]: # outputs Optional[cv2.Mat] which is a np.ndarray
        """Read camera sensor using open cv"""
        ret, image = self.camera.read()
        if not ret:
            return None
        return image
    
    def calc_fps(self) -> float:
        self.fps_interval_state['count'] += 1 # temporarily count the frames 
        current_time = time.time()
        elapsed = current_time - self.fps_interval_state['start_time']
        current_fps = 0

        if elapsed > 1.0: # want to check if a second has passed
            current_fps = self.fps_interval_state['count'] / elapsed
            self.fps_interval_state = {'count': 0, 'start_time': current_time} # reset bc we're past one second
            return current_fps
        else:
            current_fps = self.fps_interval_state['count'] / elapsed
            # dont reset temp count because we are still working in under a second
            return current_fps
    
    def slowdown_for_target_fps(self, current_time, last_frame_time) -> None:
        # Maintain target FPS
        if current_time - last_frame_time < self.fps_frame_interval:
            time.sleep(0.001)  # Short sleep to prevent CPU doing extra unnecessary operations faster than needed, given target fps (CPU spinning)
    
    def run(self):
        """Main loop streaming sensor data"""
        if not self.setup_camera():
            return
        
        last_frame_time = time.time()
        while not self.stopped():
            current_time = time.time()
            self.slowdown_for_target_fps(current_time, last_frame_time) # throttling
            sensor_capture = self.capture_sensor() # get raw data
            self.fps = self.calc_fps() # set fps
            camera_frame = CameraFrame(frame=sensor_capture)
            if not self.buffer.write_frame(camera_frame):  # update buffer with new CameraFrame object
                print("Failed to write frame to buffer, stopping camera")
                break
            last_frame_time = current_time  

        print("Camera stream ending cleanly...")
    
    """
    def start()

    using default threading.Thread method for starting the thread
    """

    def close(self):
        """Clean shutdown"""
        self.stop_camera()
        self.join(timeout=2) # cleanly finish thread
        if self.camera:
            self.camera.release()
    
    
      