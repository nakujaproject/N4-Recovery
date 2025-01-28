import serial
import threading
import logging
import json
from flask import Flask, render_template, jsonify
from flask_cors import CORS  # Import CORS

# Set up logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(message)s')

# Specify the serial port and baud rate
serial_port = 'COM5'  # Or '/dev/ttyUSB0' on Linux/macOS
baud_rate = 115200
app = Flask(__name__)

# Enable CORS for the Flask app
CORS(app)  # This will allow all domains to access the resources

# Open the serial port once at the start
ser = serial.Serial(serial_port, baud_rate, timeout=1)

# To store the latest data from the serial port (as a dictionary for better structure)
latest_data = {}

# Function to continuously read from the serial port and parse telemetry data
def read_serial_data():
    global latest_data
    while True:
        if ser.in_waiting > 0:  # Check if there is data in the buffer
            line = ser.readline().decode('utf-8').strip()
            logging.info(f"Received: {line}")  # Thread-safe logging

            # Parse telemetry data from the received string
            parse_telemetry_data(line)
            # state: receivedData.state,
            # operationMode: receivedData.operation_mode,
            # latitude: receivedData.gps_data.latitude,
            # longitude: receivedData.gps_data.longitude,
            # altitude: receivedData.gps_data.gps_altitude,
            # pressure: receivedData.alt_data.pressure,
            # temperature: receivedData.alt_data.temperature,
            # pyroDrogue: receivedData.chute_state.pyro1_state,
            # pyroMain: receivedData.chute_state.pyro2_state,
            # batteryVoltage: receivedData.battery_voltage,
# Function to parse telemetry data from the serial string
def parse_telemetry_data(line):
    global latest_data
    if "Altitude" in line:
        latest_data['gps_data.gps_altitude'] = float(line.split(":")[1].strip())
    elif "Latitude" in line:
        latest_data['gps_data.latitude'] = float(line.split(":")[1].strip())
    elif "temperature" in line:
        latest_data['alt_data.temperature'] = float(line.split(":")[1].strip())
    elif "Timestamp" in line:
        latest_data['timestamp'] = float(line.split(":")[1].strip())
    elif "state" in line:
        latest_data['state'] = int(line.split(":")[1].strip())
    elif "gx" in line:
        latest_data['gps_data.longitude'] = int(line.split(":")[1].strip())
    elif "gy" in line:
        latest_data['gy'] = int(line.split(":")[1].strip())
    elif "gz" in line:
        latest_data['gz'] = int(line.split(":")[1].strip())
    elif "longitude" in line:
        latest_data['longitude'] = float(line.split(":")[1].strip())
    elif "filtered_a" in line:
        latest_data['filtered_a'] = int(line.split(":")[1].strip())
    elif "filtered_v" in line:
        latest_data['filtered_v'] = int(line.split(":")[1].strip())
    elif "filtered_s" in line:
        latest_data['filtered_s'] = int(line.split(":")[1].strip())
    elif "ax" in line:
        latest_data['ax'] = int(line.split(":")[1].strip())
    elif "ay" in line:
        latest_data['ay'] = int(line.split(":")[1].strip())
    elif "az" in line:
        latest_data['az'] = int(line.split(":")[1].strip())
    elif "rssi" in line:
       latest_data['rssi'] = int(line.split(":")[1].strip())
    elif "antenna" in line:
       latest_data['antenna_port'] = int(line.split(":")[1].strip())
    # You can add more parsing logic if needed for other parameters

# Start the serial data reading in a separate thread
serial_thread = threading.Thread(target=read_serial_data, daemon=True)
serial_thread.start()

@app.route('/')
def index():
    return render_template('index.html')  # Simple dashboard template

@app.route('/data')
def get_data():
    if latest_data:
        return jsonify({'data': latest_data})  # Send structured telemetry data as JSON
    return jsonify({'data': 'No data available'})

if __name__ == '__main__':
    # Disable Flask's auto-reloading feature to avoid opening the serial port multiple times
    app.run(debug=True, use_reloader=False)
