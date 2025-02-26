# Robot Camera Streaming System

## Overview

This repository contains a Python-based robot system that captures camera frames and streams them to a base station over a network connection. The system is designed with a focus on efficient thread management, buffer handling, and network communication.

## System Architecture

The system consists of several key components:

### Camera Module

- Captures video frames using OpenCV
- Runs in a separate thread to prevent blocking
- Maintains frame rate control
- Creates structured `CameraFrame` objects with metadata

### Buffer System

- Implements two buffer strategies:
  - **DoubleBuffer**: Alternates between two buffers for concurrent read/write operations
  - **QueueBuffer**: Maintains separate queues for reading and writing with a swapping mechanism
- Prevents data races and provides thread-safe access to frames

### Base Station

- Manages network connection to a remote server
- Transmits camera data along with other potential payloads (LIDAR, robot health)
- Runs in a separate thread for non-blocking operation

### Processing Utilities

- Serializes camera frames for network transmission
- Uses Python's `pickle` for object serialization and `struct` for binary packing

## Dependencies

```
opencv-python==4.11.0.86
```

## Usage

The main application initializes and connects all components:

```python
def main():
    # setup camera
    camera_buffer_config = DoubleBufferConfig()
    camera_buffer = DoubleBuffer(camera_buffer_config)
    camera_config = CameraConfig(buffer=camera_buffer)
    camera = Camera(camera_config)

    # setup base station
    base_config = BaseStationConfig(base_host='192.168.50.129', base_port=2025)
    base = BaseStation(base_config)

    try:
        if base.connect_to_base():
            print("Connected to base station")
            camera.start()
            base.start()

            while True: # main robot loop
                frame = camera.buffer.read_frame()
                if frame:
                    camera_transmission = Processing.process_camera_frame(frame)
                    base.transmission_package.camera_payload = camera_transmission
                time.sleep(0.005)  # Small sleep to prevent CPU spinning

    except KeyboardInterrupt:
        print("Robot stopped by user")
    finally:
        camera.close()
        base.close()
```

## Key Classes

### Camera

```python
class Camera(threading.Thread):
    # Handles camera initialization, frame capture, and processing
```

### DoubleBuffer

```python
class FrameBuffer(ABC):
    # Abstract base class for buffer implementations
```

### DoubleBuffer

```python
class DoubleBuffer(FrameBuffer):
    # Implements double-buffering strategy with buffer switching
```

### QueueBuffer

\*currently not used at the momement\*

```python
class QueueBuffer(FrameBuffer):
    # Implements queue-based buffering with a swap thread
```

### BaseStation

```python
class BaseStation(threading.Thread):
    # Manages network connection and data transmission
```

## Configuration

The system uses dataclasses for configuration:

- `CameraConfig`: Controls camera parameters like FPS and buffer settings
- `DoubleBufferConfig`: Sets buffer timeout values
- `QueueBufferConfig`: Configures queue size and timeout
- `BaseStationConfig`: Sets network host and port

## Thread Management

The system uses Python's threading module to manage concurrent operations:

- Camera capture runs in a dedicated thread
- Base station communication runs in a separate thread
- Buffer systems use synchronization primitives (locks, events) to coordinate access

## Notes

- The system requires a `shared.py` file containing the `CameraFrame` class definition
- The network protocol uses a simple size-prefixed binary format for sending frames
- Performance monitoring includes FPS calculation and tracking
