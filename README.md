# sand-dunes-rover

Spring 2025
Arapahoe Community College Robotics Club - rover project

open cv documentation:
https://docs.opencv.org/4.11.0/index.html
(use the in-page google search bar at the top to search for keywords, methods, objects, etc)

# VideoServer Class Documentation

The VideoServer class implements a multi-threaded server that accepts video frames from a client, processes them, and optionally applies machine learning (ML) inference, followed by displaying annotated frames.

## Classes

### FrameBuffer

A thread-safe buffer to store frames with a size limit. It uses a queue.Queue to handle frame management and ensure synchronization.

Methods
**init**(self, maxsize: int = 90)
Initializes a frame buffer with a maximum size.

put(self, frame: Frame) -> None
Adds a frame to the buffer, dropping the oldest frame if the buffer is full.

get(self) -> Optional[Frame]
Retrieves the next frame from the buffer.

Constructor
**init**(self, host: str, port: int)
Initializes the server with the provided host and port.
host (str): The server's IP address (e.g., "0.0.0.0" to listen on all interfaces).
port (int): The port on which the server listens for client connections.

### VideoServer

A server that handles receiving, processing, and displaying video frames. It operates in a separate thread and communicates with a client over a socket.

Methods
setup_server(self) -> None
Sets up the server socket to listen for incoming connections.

accept_connection(self) -> tuple
Accepts a connection from a client, returning the client's address.

receive_frame_size(self) -> Optional[int]
Receives the size of the next frame from the client.

receive_frame_data(self, msg_size: int) -> Optional[bytes]
Receives frame data of a specified size.

process_frame(self, frame_data: bytes) -> bool
Processes the received frame, including optional machine learning (ML) inference and adding the frame to the buffer.

draw_annotations(self, frame: Frame) -> any
Draws annotations (e.g., FPS and ML results) on the frame for visualization.

display_frames(self, window_name: str = 'Robot Camera Feed') -> None
Displays frames in an OpenCV window with annotations.

connect(self) -> None
Starts the server and waits for a client to connect.

run(self) -> None
Main server loop that continuously receives, processes, and displays frames until the server is stopped.

stop(self) -> None
Stops the server, closes sockets, and cleans up resources.

## Future Extensions

ML Model Integration: The ml_model attribute is optional, and if provided, it can run inference on the received frames. You can extend this by adding specific ML model handling and more advanced annotations.

Frame Buffering: The FrameBuffer class currently supports simple frame dropping when the buffer is full. You could extend this to allow more sophisticated buffer management strategies if needed.

## Dependencies

cv2 (OpenCV): Used for frame manipulation and display.
pickle: Used to serialize and deserialize frames.
socket: Used for communication between the server and client.
struct: Used for packing and unpacking frame sizes.
queue: Provides thread-safe frame storage.
threading: Allows the server to run in a separate thread.
