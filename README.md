# Sand Dunes Rover Project

Spring 2025
Arapahoe Community College Robotics Club - rover project

## Overview

This project implements a remote-controlled rover system designed for sand dune environments. The system consists of two main components:

1. A mobile **Rover** with camera and LIDAR sensors
2. A **Base Station** that receives and displays sensor data from the rover

The system uses a client-server architecture where the rover connects to the base station and streams sensor data over a TCP/IP connection.

## System Architecture

### Rover Component

The rover is the mobile platform equipped with:

- Camera for real-time video feed
- LIDAR sensor for distance measurements and obstacle detection
- Communication module for data transmission to the base station

### Base Station Component

The base station receives and processes data from the rover, including:

- Displaying the video feed
- Visualizing LIDAR data
- Monitoring telemetry and system status

## Features

- **Real-time Video Streaming**: Camera frames are compressed and transmitted to the base station for display
- **LIDAR Data Processing**: Distance measurements from the LIDAR sensor are sent to the base station for visualization
- **Efficient Data Buffering**: Double buffering system ensures smooth data flow between components
- **Thread-safe Communication**: Multi-threaded design for parallel data capture and transmission
- **Packet Processing**: Custom protocol for reliable data exchange between rover and base station
- **Data Visualization**: Real-time display of camera feed and LIDAR readings

## Getting Started

### Prerequisites

- Python 3.7+
- OpenCV (`cv2`)
- NumPy
- pySerial

### Installation

1. Clone the repository:

```bash
git clone https://github.com/your-username/sand-dunes-rover.git
cd sand-dunes-rover
```

2. Create and activate a virtual environment:

```bash
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

3. Install dependencies:

```bash
pip install -r requirements.txt
```

### Project Structure

```
sand-dunes-rover/
├── base_station/         # Base station code
│   └── main.py           # Base station entry point
├── robot/                # Rover code
│   ├── BaseStation.py    # Communication with base station
│   ├── Buffer.py         # Data buffering utilities
│   ├── Camera.py         # Camera sensor interface
│   ├── Util.py           # Utility functions
│   └── main.py           # Rover entry point
└── shared/               # Shared code between rover and base station
    └── Communication.py  # Message definitions and communication protocols
```

## Usage

### Running the Base Station

```bash
cd sand-dunes-rover
source venv/bin/activate  # On Windows: venv\Scripts\activate
cd base_station
python main.py
```

This starts the base station, which listens for connections on port 2025 by default.

### Running the Rover

```bash
cd sand-dunes-rover
source venv/bin/activate  # On Windows: venv\Scripts\activate
cd robot
python main.py
```

This starts the rover components, which will attempt to connect to the base station.

### Base Station Controls

- **Q**: Quit the application
- **L**: Toggle LIDAR display
- **C**: Toggle camera display

## Communication Protocol

The system uses a custom message protocol with the following message types:

1. **CAMERA_FRAME**: Contains compressed video frames
2. **LIDAR_DATA**: Contains distance readings from the LIDAR sensor
3. **TELEMETRY**: Contains rover status information
4. **COMMAND**: For sending commands to the rover

Each message includes:

- Message type identifier
- Timestamp
- Serialized payload data

## LIDAR Sensor

The system is configured to work with a rotational LIDAR sensor that provides distance measurements. The LIDAR data is parsed and processed to extract:

- Distance readings at various angles
- Intensity values
- Angular resolution

## Extending the System

### Adding New Sensors

To add a new sensor:

1. Create a new sensor interface class similar to `Camera.py`
2. Define appropriate message types in `shared/Communication.py`
3. Update the rover's main loop to capture and transmit the new sensor data
4. Update the base station to receive and visualize the new data

### Adding Remote Control

To add remote control capabilities:

1. Define command message types in `Communication.py`
2. Implement command handling on the rover
3. Add a user interface on the base station for sending commands

## Troubleshooting

### Connection Issues

- Verify that the base station and rover are on the same network
- Check firewall settings to ensure port 2025 is open
- Verify the correct IP address is configured in `BaseStationConfig`

### Video Feed Issues

- Check that the camera is properly connected and recognized
- Verify that OpenCV is properly installed
- Adjust the camera resolution and frame rate in `Camera.py` if performance is an issue

### LIDAR Issues

- Check the serial connection to the LIDAR sensor
- Verify the correct port is specified in the LIDAR initialization
- Adjust the baud rate if necessary
