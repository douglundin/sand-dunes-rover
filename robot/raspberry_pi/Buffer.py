from abc import ABC, abstractmethod
import threading
import queue
from typing import Any, Optional
from dataclasses import dataclass

class FrameBuffer(ABC): # abstract base class, never directly used
    @abstractmethod
    def write_frame(self, frame: Any) -> bool:
        """Write a frame to the buffer"""
        pass
    
    @abstractmethod
    def read_frame(self) -> Optional[Any]:
        """Read a frame from the buffer"""
        pass
    
    @abstractmethod
    def start(self) -> None:
        """Start the buffer processing"""
        pass
    
    @abstractmethod
    def stop(self) -> None:
        """Stop the buffer processing"""
        pass

@dataclass
class DoubleBufferConfig:
    timeout: float = 1.0

class DoubleBuffer(FrameBuffer): 
    def __init__(self, config: DoubleBufferConfig):
        self.buffer_a = bytearray()
        self.buffer_b = bytearray()
        self.writing_to_a = True
        """important - alternates which buffer is the read 
        buffer and which is the write, eliminating copy 
        operation if using fixed dedicated read/write buffers"""
        
        self.read_done = threading.Event()
        self.write_done = threading.Event()
        self.lock = threading.Lock()
        self.running = False
        
        self.read_done.set()  # Initially allow writing
        self.config = config

    def write_frame(self, frame: Any) -> bool:
        if not self.running:
            return False
            
        self.read_done.wait(timeout=self.config.timeout) # wait to emit a 'finished reading' flag
        
        with self.lock:
            if self.writing_to_a:
                self.buffer_a = frame
            else:
                self.buffer_b = frame
                
            self.write_done.set()
            self.read_done.clear()
            return True

    def read_frame(self) -> Optional[Any]:
        if not self.running:
            return None
            
        if not self.write_done.wait(timeout=self.config.timeout):
            return None
            
        with self.lock:
            data = self.buffer_b if self.writing_to_a else self.buffer_a
            self.read_done.set()
            self.write_done.clear()
            self.writing_to_a = not self.writing_to_a
            return data

    def start(self) -> None:
        self.running = True

    def stop(self) -> None:
        self.running = False
        self.read_done.set()
        self.write_done.set()

@dataclass
class QueueBufferConfig:
    max_size: int = 32
    timeout: float = 1.0

class QueueBuffer(FrameBuffer):
    def __init__(self, config: QueueBufferConfig):
        self.write_queue = queue.Queue(maxsize=config.max_size)
        self.read_queue = queue.Queue(maxsize=config.max_size)
        self.running = False
        self.config = config
        self.swap_thread = None

    def write_frame(self, frame: Any) -> bool: # false = unsuccessful write
        if not self.running:
            return False
            
        try:
            self.write_queue.put(frame, block=False)
            return True
        except queue.Full:
            return False

    def read_frame(self) -> Optional[Any]:
        if not self.running:
            return None
            
        try:
            return self.read_queue.get(timeout=self.config.timeout)
        except queue.Empty:
            return None

    def _buffer_swap_thread(self) -> None:
        while self.running:
            try:
                frame = self.write_queue.get(timeout=0.1)
                self.read_queue.put(frame, timeout=0.1)
            except (queue.Empty, queue.Full):
                continue

    def start(self) -> None:
        self.running = True
        self.swap_thread = threading.Thread(
            target=self._buffer_swap_thread,
            daemon=True
        )
        self.swap_thread.start()

    def stop(self) -> None:
        self.running = False
        if self.swap_thread:
            self.swap_thread.join(timeout=1.0)

