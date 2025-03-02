import sys
import os
# modifies the sys.path to access directory 'shared/', two levels up from the current file's directory.
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

import time
import Util

from BaseStation import BaseStation, BaseStationConfig
from Camera import Camera, CameraConfig
from Buffer import DoubleBuffer, DoubleBufferConfig 




def main():
    # setup camera
    camera_buffer_config = DoubleBufferConfig() # standard buffer configuration
    camera_buffer = DoubleBuffer(camera_buffer_config)
    camera_config = CameraConfig(buffer=camera_buffer) 
    camera = Camera(camera_config) 

    # setup base_station station
    base_config = BaseStationConfig(
        # base_host='192.168.50.129', 
        base_port=2025)
    base_station = BaseStation(base_config)
    
    try:
        if base_station.connect_to_base():
            print("Connected to base_station station")
            camera.start()
            base_station.start()
            Util.get_thread_info() # logging to see the threads
            while True: # main robot loop
                frame = camera.buffer.read_frame() # get current frame from camera in thread safe way (because of buffer)
                if frame:
                    base_station.set_camera_frame(frame)
                time.sleep(0.005) # throttling
                       
    except KeyboardInterrupt:
        print("Robot stopped by user")
    finally:
        camera.close()
        base_station.close()

if __name__ == "__main__":
    main()