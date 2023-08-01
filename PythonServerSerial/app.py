from flask import Flask, jsonify
from serial_interface import SerialInterface
import threading

app = Flask(__name__)
serial_interface = SerialInterface('/dev/ttyUSB0', 9600)

# Variable to store the most recent data from the SerialInterface
latest_data = {}

# Function to continuously update the latest_data variable with the most recent data
def update_data():
    global latest_data
    while True:
        data = serial_interface.read()
        latest_data = data

# Create and start a separate thread for updating the latest_data
update_thread = threading.Thread(target=update_data)
update_thread.daemon = True
update_thread.start()

@app.route('/data', methods=['GET'])
def get_latest_data():
    return jsonify(latest_data)

if __name__ == '__main__':
    try:
        app.run(host='0.0.0.0', port=5000)
    except KeyboardInterrupt:
        serial_interface.close()