import csv
import random
import math

# Constants for simulation
EXPECTED_APOGEE = 3000  # meters
DESCENT_RATE = (30, 50)  # meters per second
GRAVITY = 9.81  # meters per second squared

# Define the ranges for the random data generation
acceleration_range = (-15, 15)
velocity_range = (0, 300)  
orientation_range = (0, 360)  
pressure_range = (300, 1100)  

# Number of data points to generate
num_data_points = 50

# File name for the CSV
csv_file_name = "fake_flight_data.csv"

def generate_fake_flight_data():
    with open(csv_file_name, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write the header
        writer.writerow(['timestamp', 'velocity', 'altitude', 'acceleration', 'orientation', 'air_pressure'])
        
        for i in range(num_data_points):
            timestamp = i  # Simple sequential timestamp
            
            if i < 5:
                # Pre-flight data
                velocity = 0
                altitude = 0
                acceleration = 0
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(*pressure_range)
            elif i < 13:
                # Powered flight 
                altitude = (i - 5) * (EXPECTED_APOGEE / 10)
                velocity = random.uniform(100, 300)
                acceleration = random.uniform(5, 15)
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(300, 400)
            elif i < 25:
                # Coasting 
                altitude = (EXPECTED_APOGEE / 2) + (i * 75)
                velocity = random.uniform(80, 0)  
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(300, 400)
            elif i == 25:
                # Drogue deploy (at apogee)
                altitude = EXPECTED_APOGEE  
                velocity = 0
                acceleration = 0  
                orientation = 0
                air_pressure = random.uniform(300, 400)
            elif i < 40:
                # Drogue descent
                altitude = (EXPECTED_APOGEE / 2) - (i * 25)
                velocity = -random.uniform(*DESCENT_RATE) 
                acceleration = random.uniform(-GRAVITY, -5)
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(300, 400)
            elif i == 40:
                # Main chute deploy
                altitude = 450
                velocity = -random.uniform(20, 30)  
                acceleration = random.uniform(-5, 5)
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(400, 700)
            elif i < 45:
                # Main descent
                altitude = (EXPECTED_APOGEE / 2) - (i * 30)
                velocity = -random.uniform(10, 20)  
                acceleration = random.uniform(-5, 5)
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(600, 900)
            else:
                # Post flight
                altitude = 0
                velocity = 0
                acceleration = random.uniform(GRAVITY, 2 * GRAVITY)
                orientation = random.uniform(*orientation_range)
                air_pressure = random.uniform(1000, 1100)
            
            writer.writerow([timestamp, velocity, altitude, acceleration, orientation, air_pressure])

    print(f"Fake flight data written to {csv_file_name}")

# Generate the fake flight data
generate_fake_flight_data()
