from dataclasses import dataclass
from typing import Any, Dict, Optional
import time

@dataclass
class Frame:
    """Shared frame format between robot and base station"""
    image: Any  # The raw image data
    timestamp: float  # Capture timestamp
    frame_id: int  # Unique identifier
    metadata: Dict[str, Any]  # For ML inference results and other data
    
    @classmethod
    def create(cls, image: Any, frame_id: int) -> 'Frame':
        return cls(
            image=image,
            timestamp=time.time(),
            frame_id=frame_id,
            metadata={}
        )
