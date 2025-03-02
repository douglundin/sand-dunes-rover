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

# Robotics Fundamentals: Sensors, Control Systems, and Movement

This guide introduces key concepts in robotics for programmers new to the field. It covers how robots sense their environment, process that information, and move in response.

## Sensors: The Robot's Senses

Sensors are how robots perceive the world. Different sensors provide different types of information:

### Distance and Proximity Sensors

- **LIDAR** (Light Detection and Ranging)

  - Uses laser beams to create detailed distance maps
  - Provides precise measurements at many angles
  - Good for mapping and obstacle detection
  - Examples: LD19, RPLidar, Velodyne

- **Ultrasonic Sensors**

  - Use sound waves to measure distance
  - Simpler and cheaper than LIDAR
  - Less precise but effective for basic obstacle avoidance
  - Examples: HC-SR04, MaxBotix

- **Infrared (IR) Proximity**
  - Detect nearby objects using infrared light
  - Good for short-range detection (typically <30cm)
  - Can be affected by ambient light
  - Examples: Sharp GP2Y0A21, TCRT5000

### Motion and Position Sensors

- **IMU** (Inertial Measurement Unit)

  - Combines accelerometer, gyroscope, and sometimes magnetometer
  - Measures acceleration, orientation, and heading
  - Critical for balance and navigation
  - Examples: MPU-6050, BNO055

- **Encoders**

  - Measure wheel or motor rotation
  - Help track movement distance and speed
  - Types: optical, magnetic, absolute, incremental
  - Examples: Quadrature encoders, optical rotary encoders

- **GPS**
  - Provides global position data
  - Works outdoors with clear sky view
  - Limited precision (±2-10 meters)
  - Examples: NEO-6M, ZED-F9P (RTK GPS, higher precision)

### Environmental Sensors

- **Cameras**

  - Provide rich visual information
  - Enable object recognition, tracking, mapping
  - Stereo cameras add depth perception
  - Examples: Raspberry Pi Camera, Intel RealSense

- **Force/Pressure Sensors**

  - Detect physical contact or weight
  - Used for grip control and collision detection
  - Examples: FSR (Force Sensing Resistor), load cells

- **Temperature/Humidity Sensors**
  - Monitor environmental conditions
  - Protect electronics from extreme conditions
  - Examples: DHT22, BME280

## Computer Control: The Robot's Brain

How robots process sensor data and make decisions:

### Microcontrollers vs. Single Board Computers

- **Microcontrollers** (Arduino, STM32, ESP32)

  - Handle low-level control and timing-critical tasks
  - Directly interface with motors and simple sensors
  - Real-time operation with minimal latency
  - Limited processing power for complex algorithms

- **Single Board Computers** (Raspberry Pi, Jetson Nano)
  - Run full operating systems
  - Process complex sensor data (vision, LIDAR)
  - Handle high-level planning and decision making
  - Connect to networks and cloud services

### Control Architectures

- **Reactive Control**

  - Direct mapping from sensors to actions
  - Simple, fast responses to environmental changes
  - Example: Obstacle detected → Turn away
  - Minimal internal state or planning

- **Deliberative Control**

  - Builds internal models and plans
  - Considers multiple steps ahead
  - More complex but handles challenging scenarios
  - Example: Path planning through a maze

- **Hybrid Control**
  - Combines reactive for safety and deliberative for efficiency
  - Layered approach with different response times
  - Most modern robots use some form of hybrid control

### Control Loops

- **Open Loop Control**

  - Sends commands without feedback
  - Simple but vulnerable to disturbances
  - Example: "Turn motors on for 2 seconds to move forward"

- **Closed Loop Control**

  - Uses sensor feedback to adjust commands
  - Much more precise and robust
  - Example: "Move forward 1 meter, adjusting power based on encoder feedback"

- **PID Control**
  - Proportional-Integral-Derivative controller
  - Widely used feedback control method
  - Adjusts output based on error magnitude, accumulation, and rate of change
  - Example: Maintaining robot balance or precise motor speed

## Movement: How Robots Interact with the World

Different locomotion systems and their characteristics:

### Wheeled Systems

- **Differential Drive**

  - Two independently controlled drive wheels
  - Simple, efficient, and maneuverable
  - Can rotate in place by spinning wheels in opposite directions
  - Examples: Most educational robots, vacuum robots

- **Car-like (Ackermann) Steering**

  - Uses separate steering and drive mechanisms
  - More efficient at higher speeds
  - Limited turning radius (can't rotate in place)
  - Examples: Autonomous cars, some outdoor robots

- **Omnidirectional Drive**
  - Can move in any direction without rotating first
  - Uses special wheels (mecanum, omniwheels)
  - More complex but extremely maneuverable
  - Examples: Factory robots, some service robots

### Legged Systems

- **Bipedal (Two-legged)**

  - Human-like locomotion
  - Complex balance and control requirements
  - Navigates diverse terrain
  - Examples: Humanoid robots like Atlas or ASIMO

- **Quadrupedal (Four-legged)**
  - Animal-like locomotion
  - More stable than bipedal
  - Good for rough terrain
  - Examples: Spot (Boston Dynamics), ANYmal

### Actuators: Creating Movement

- **DC Motors**

  - Common in wheeled robots
  - Simple control with varying speed and direction
  - Often used with gearboxes to increase torque

- **Servo Motors**

  - Precise position control
  - Limited rotation range (typically 180° or 270°)
  - Common in robot arms and steering

- **Stepper Motors**

  - Move in discrete steps for precise positioning
  - Open-loop position control possible
  - Common in 3D printers and precision systems

- **Linear Actuators**
  - Create straight-line motion
  - Used for lifting, pushing, or sliding mechanisms

## Putting It All Together: Control Flow

In a typical robot system:

1. **Sensing**: Sensors gather environmental data
2. **Perception**: Raw sensor data is processed into meaningful information
3. **Planning**: The system decides what actions to take based on goals and perception
4. **Control**: Commands are sent to actuators
5. **Feedback**: New sensor readings measure the results of actions
6. **Adaptation**: The system adjusts plans based on feedback

This cycle repeats continuously, often at different rates for different subsystems.

## Common Programming Patterns

- **Publisher-Subscriber**

  - Components publish data to topics
  - Other components subscribe to relevant topics
  - Decouples data producers from consumers
  - Common in ROS (Robot Operating System)

- **State Machines**

  - Define discrete states and transitions
  - Clear behavior organization
  - Good for robots with distinct operational modes

- **Behavior Trees**
  - Hierarchical structure of tasks and behaviors
  - Flexible, modular approach to complex behaviors
  - Combines reactivity with goal-directed behavior

## Practical Considerations

- **Power Management**

  - Battery capacity limits operation time
  - Different components have different power needs
  - Power spikes during motor operation can affect sensors

- **Communication Latency**

  - Time delays between sensing and action
  - Critical for real-time control
  - Affects safety and performance

- **Error Handling**
  - Sensor failures must be detected and managed
  - Graceful degradation when systems fail
  - Safety fallbacks for critical operations

## Getting Started

1. Begin with a simple platform (wheeled robot kit)
2. Master basic sensor integration and motor control
3. Implement simple reactive behaviors
4. Add more complex planning and perception gradually
5. Explore ROS for larger, more complex systems

Remember that robotics is inherently multidisciplinary - combining mechanical, electrical, and software aspects. The best learning comes from hands-on experimentation and iterative improvement.
