import matplotlib.pyplot as plt
import numpy as np
import math
import csv
import os.path
import argparse

def visualize_from_file(filename):
    """
    Visualize LIDAR data from a CSV file
    
    Args:
        filename: Path to CSV file with angle,distance,intensity data
    """
    if not os.path.exists(filename):
        print(f"Error: File {filename} not found!")
        return
    
    # Read data from CSV file
    angles = []
    distances = []
    intensities = []
    
    with open(filename, 'r') as f:
        reader = csv.reader(f)
        next(reader)  # Skip header row
        for row in reader:
            if len(row) >= 3:
                angles.append(float(row[0]))
                distances.append(float(row[1]))
                intensities.append(float(row[2]))
    
    print(f"Loaded {len(angles)} data points from {filename}")
    
    # Create visualizations
    create_polar_plot(angles, distances, intensities)
    create_cartesian_plot(angles, distances, intensities)
    create_intensity_plot(angles, distances, intensities)
    plt.show()

def create_polar_plot(angles, distances, intensities):
    """Create a polar plot showing distance measurements around the sensor"""
    plt.figure(figsize=(10, 10))
    ax = plt.subplot(111, projection='polar')
    
    # Convert angles to radians for polar plot
    angles_rad = [math.radians(a) for a in angles]
    
    # Use colormap to represent intensity
    sc = ax.scatter(angles_rad, distances, c=intensities, cmap='viridis', 
                   s=5, alpha=0.7)
    
    # Add a colorbar to show intensity scale
    cbar = plt.colorbar(sc)
    cbar.set_label('Intensity')
    
    # Set up plot labels and title
    ax.set_title('LIDAR Scan - Polar View')
    ax.set_theta_zero_location("N")  # 0 degrees at the top
    ax.set_theta_direction(-1)  # Clockwise direction
    ax.set_rmax(max(distances) * 1.1)  # Set radial limit with some margin
    
    # Add gridlines for distance reference
    ax.grid(True)

def create_cartesian_plot(angles, distances, intensities):
    """Create a cartesian plot showing x-y coordinates"""
    plt.figure(figsize=(10, 8))
    
    # Convert polar (angle, distance) to cartesian (x, y)
    x_values = []
    y_values = []
    
    for angle, distance in zip(angles, distances):
        # Convert to radians and calculate x, y
        angle_rad = math.radians(angle)
        x = distance * math.cos(angle_rad)
        y = distance * math.sin(angle_rad)
        x_values.append(x)
        y_values.append(y)
    
    # Create scatter plot with intensity coloring
    sc = plt.scatter(x_values, y_values, c=intensities, cmap='viridis', 
                   s=5, alpha=0.7)
    
    # Add a colorbar
    cbar = plt.colorbar(sc)
    cbar.set_label('Intensity')
    
    # Set up plot labels and title
    plt.title('LIDAR Scan - Cartesian View')
    plt.xlabel('X Distance (mm)')
    plt.ylabel('Y Distance (mm)')
    plt.axis('equal')  # Equal scaling for x and y
    plt.grid(True)
    
    # Add LIDAR position marker at origin
    plt.plot(0, 0, 'ro', markersize=10)
    plt.annotate('LIDAR', (0, 0), textcoords="offset points", 
                xytext=(0,10), ha='center')

def create_intensity_plot(angles, distances, intensities):
    """Create a plot showing intensity vs angle"""
    plt.figure(figsize=(12, 6))
    
    # Sort by angle for connected line plot
    sorted_data = sorted(zip(angles, intensities))
    sorted_angles, sorted_intensities = zip(*sorted_data)
    
    plt.plot(sorted_angles, sorted_intensities, '-', alpha=0.7)
    plt.scatter(angles, intensities, c=distances, cmap='plasma', 
               s=5, alpha=0.5)
    
    plt.title('LIDAR Intensity vs Angle')
    plt.xlabel('Angle (degrees)')
    plt.ylabel('Intensity')
    plt.grid(True)
    
    # Add colorbar for distance reference
    sm = plt.cm.ScalarMappable(cmap='plasma')
    sm.set_array(distances)
    cbar = plt.colorbar(sm)
    cbar.set_label('Distance (mm)')

def real_time_visualization():
    """
    Placeholder for real-time visualization functionality.
    Future enhancement: Implement real-time plotting as data comes in.
    """
    print("Real-time visualization not implemented yet.")
    print("For now, please run the main LIDAR code to collect data, then")
    print("use this tool to visualize the saved data.")

if __name__ == "__main__":
    # Set up command line argument parsing
    parser = argparse.ArgumentParser(description='Visualize LIDAR data')
    parser.add_argument('--file', type=str, default='lidar_data.csv',
                       help='CSV file containing LIDAR data (default: lidar_data.csv)')
    parser.add_argument('--realtime', action='store_true',
                       help='Enable real-time visualization (not implemented yet)')
    
    args = parser.parse_args()
    
    if args.realtime:
        real_time_visualization()
    else:
        visualize_from_file(args.file)