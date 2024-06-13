import csv
import matplotlib.pyplot as plt

# Data file path 
data_file = 'fake_flight_data.csv'

def generate_flight_data_graphs():
  # Read data from CSV
  data = []
  with open(data_file, 'r') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # Skip the header row
    for row in reader:
      timestamp, velocity, altitude, *_ = row  # Extract relevant data
      data.append((float(timestamp), float(velocity), float(altitude)))

  # Separate Altitude and Velocity
  timestamps, velocities, altitudes = zip(*data)

  # Plot Altitude
  plt.figure(figsize=(10, 6))
  plt.plot(timestamps, altitudes, label='Altitude (m)', color='red')
  plt.xlabel('Time (s)')
  plt.ylabel('Altitude (m)')
  plt.title('Altitude vs. Time')
  plt.grid(True)

  # Plot Velocity (on the same figure)
  plt.plot(timestamps, velocities, label='Velocity (m/s)', color='green')
  plt.legend()  # Update legend to include both lines

  plt.show()

if __name__ == "__main__":
  generate_flight_data_graphs()
