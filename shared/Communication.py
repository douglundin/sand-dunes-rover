from dataclasses import dataclass, field
from typing import Dict, Any, Optional
from enum import Enum
import time
import json
import cv2
import numpy as np
import pickle

class MessageType(Enum):
    CAMERA = 1
    LIDAR = 2
    STATUS = 3 # robot status and metrics
    COMMAND = 4

@dataclass
class Message:
    """Base message class for robot-base communication"""
    timestamp: float = field(default_factory=lambda: time.time())
    message_type: Optional[MessageType] = None
    
    def serialize(self) -> bytes:
        """Default serialization using pickle"""
        return pickle.dumps(self)
    
    @classmethod
    def deserialize(cls, data: bytes):
        """Default deserialization using pickle"""
        return pickle.loads(data)

@dataclass
class CameraFrame(Message):
    """OpenCV camera frame message"""
    frame: Optional[np.ndarray] = None  # OpenCV image as numpy array
    metadata: Dict[str, Any] = field(default_factory=dict)  # Frame-specific metadata
    
    def __post_init__(self):
        self.message_type = MessageType.CAMERA
    
    def serialize(self) -> bytes:
        """Serialize with frame compression using OpenCV"""
        if self.frame is not None:
            # Create a data dictionary with core fields and metadata
            data_dict = {
                'timestamp': self.timestamp,
                'message_type': self.message_type.value,
                'metadata': self.metadata
            }
            
            # Encode frame as JPG
            _, encoded_frame = cv2.imencode('.jpg', self.frame)
            
            # Return both parts
            return json.dumps(data_dict).encode('utf-8') + b'|FRAME|' + encoded_frame.tobytes()
        else:
            return super().serialize()  # fallback to pickle if no frame
    
    @classmethod
    def deserialize(cls, data: bytes):
        """Deserialize with frame decompression"""
        if b'|FRAME|' in data:
            # Split the data into header and frame parts
            header_part, frame_part = data.split(b'|FRAME|', 1)
            
            # Parse the JSON header
            data_dict = json.loads(header_part.decode('utf-8'))
            
            # Extract fields
            timestamp = data_dict.get('timestamp', time.time())
            message_type = MessageType(data_dict.get('message_type'))
            metadata = data_dict.get('metadata', {})
            
            # Decode the frame
            frame_array = np.frombuffer(frame_part, dtype=np.uint8)
            frame = cv2.imdecode(frame_array, cv2.IMREAD_COLOR)
            
            # Create the CameraFrame object
            return cls(
                timestamp=timestamp,
                message_type=message_type,
                frame=frame,
                metadata=metadata
            )
        else:
            return super().deserialize(data)  # fallback to pickle if no frame marker
        
@dataclass
class LidarData(Message):
    """Robot sensor data message"""
    # this all will change once we learn how the lidar data is structured
    readings: Dict[str, float] = field(default_factory=dict)
    
    def __post_init__(self):
        self.message_type = MessageType.LIDAR

@dataclass
class StatusData(Message):
    status: Dict[str, Any] = field(default_factory=dict)  # robot status-specific metadata

    def __post_init__(self):
        self.message_type = MessageType.STATUS

# Simple transport mechanism
class Communication:
    @staticmethod
    def send(message: Message, connection):
        """Send message to connection"""
        data = message.serialize()
        connection.send(data)
    
    @staticmethod
    def receive(connection, expected_type=None):
        """Receive message from connection"""
        data = connection.recv()
        
        # Determine message type from first few bytes
        if data.startswith(b'{') and b'"message_type"' in data[:50]:
            json_preview = json.loads(data.split(b'|FRAME|', 1)[0])
            msg_type = MessageType(json_preview["message_type"])
        else:
            # Fallback to pickle's type information
            msg = pickle.loads(data)
            msg_type = msg.message_type
        
        # Deserialize based on type
        if msg_type == MessageType.CAMERA:
            return CameraFrame.deserialize(data)
        elif msg_type == MessageType.LIDAR_DATA:
            return LidarData.deserialize(data)
        else:
            return Message.deserialize(data)

