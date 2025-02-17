import socket
import pickle
import struct
import cv2
import threading
import queue
from typing import Optional
from shared.shared_types import Frame

class FrameBuffer:
    """Thread-safe frame buffer with size limit"""
    def __init__(self, maxsize: int = 30):
        self.frames = queue.Queue(maxsize=maxsize)
        
    def put(self, frame: Frame) -> None:
        """Add frame to buffer, dropping oldest if full"""
        try:
            self.frames.put_nowait(frame)
        except queue.Full:
            try:
                self.frames.get_nowait()  # Drop oldest
                self.frames.put_nowait(frame)
            except (queue.Empty, queue.Full):
                pass

    def get(self) -> Optional[Frame]:
        """Get next frame from buffer"""
        try:
            return self.frames.get_nowait()
        except queue.Empty:
            return None

class VideoServer(threading.Thread):
    def __init__(self, host: str, port: int):
        super().__init__(daemon=True)
        self.host = host
        self.port = port
        self.server_socket = None
        self.client_socket = None
        self.is_running = False
        self.data_buffer = b""
        self.payload_size = struct.calcsize("L")
        self.frame_buffer = FrameBuffer()
        
        # Optional ML model can be added here
        self.ml_model = None

    def setup_server(self) -> None:
        """Initialize server socket"""
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(1)
            print("Server started, waiting for connection...")
        except Exception as e:
            print(f"Server setup failed: {e}")
            self.stop()
            raise

    def accept_connection(self) -> tuple:
        """Accept client connection"""
        try:
            self.client_socket, addr = self.server_socket.accept()
            print(f"Connected to {addr}")
            return addr
        except Exception as e:
            print(f"Connection acceptance failed: {e}")
            self.stop()
            raise

    def receive_frame_size(self) -> Optional[int]:
        """Get size of next frame"""
        try:
            while len(self.data_buffer) < self.payload_size:
                packet = self.client_socket.recv(4096)
                if not packet:
                    print("Connection lost")
                    return None
                self.data_buffer += packet

            packed_msg_size = self.data_buffer[:self.payload_size]
            self.data_buffer = self.data_buffer[self.payload_size:]
            return struct.unpack("L", packed_msg_size)[0]
        except Exception as e:
            print(f"Error receiving frame size: {e}")
            return None

    def receive_frame_data(self, msg_size: int) -> Optional[bytes]:
        """Get frame data of specified size"""
        try:
            while len(self.data_buffer) < msg_size:
                self.data_buffer += self.client_socket.recv(4096)

            frame_data = self.data_buffer[:msg_size]
            self.data_buffer = self.data_buffer[msg_size:]
            return frame_data
        except Exception as e:
            print(f"Error receiving frame data: {e}")
            return None

    def process_frame(self, frame_data: bytes) -> bool:
        """Process received frame and add to buffer"""
        try:
            frame = pickle.loads(frame_data)
            
            # Run ML inference if model is loaded
            if self.ml_model is not None:
                results = self.ml_model.predict(frame.image)
                frame.metadata['ml_results'] = results
            
            self.frame_buffer.put(frame)
            return True
        except Exception as e:
            print(f"Error processing frame: {e}")
            return False

    def draw_annotations(self, frame: Frame) -> Any:
        """Draw metadata and ML results on frame"""
        annotated_frame = frame.image.copy()
        
        # Draw FPS
        if 'fps' in frame.metadata:
            cv2.putText(
                annotated_frame,
                f"FPS: {frame.metadata['fps']:.1f}",
                (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX,
                1,
                (0, 255, 0),
                2
            )
        
        # Draw ML results if present
        if 'ml_results' in frame.metadata:
            # Add visualization code for ML results here
            pass
            
        return annotated_frame

    def display_frames(self, window_name: str = 'Robot Camera Feed') -> None:
        """Display frames with annotations"""
        frame = self.frame_buffer.get()
        if frame is not None:
            annotated_frame = self.draw_annotations(frame)
            cv2.imshow(window_name, annotated_frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                self.is_running = False

    def connect(self) -> None:
        """Start server and wait for connection"""
        try:
            self.setup_server()
            self.accept_connection()
            self.is_running = True
        except Exception as e:
            print(f"Connection error: {e}")

    def run(self) -> None:
        """Main processing loop"""
        try:
            self.connect()

            while self.is_running:
                msg_size = self.receive_frame_size()
                if msg_size is None:
                    break

                frame_data = self.receive_frame_data(msg_size)
                if frame_data is None:
                    break

                if not self.process_frame(frame_data):
                    break

        except Exception as e:
            print(f"Frame processing error: {e}")
        finally:
            self.stop()

    def stop(self) -> None:
        """Clean shutdown"""
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

def main():
    server = VideoServer("0.0.0.0", 2025)

    try:
        server.start()
        while True:
            server.display_frames()
    except KeyboardInterrupt:
        print("Server stopped by user")
    finally:
        server.stop()

if __name__ == "__main__":
    main()