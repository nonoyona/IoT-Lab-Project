import csv
import time
from serial_interface import SerialInterface

def main():
    port = "COM1"  # Replace with your actual serial port
    baudrate = 9600  # Replace with the appropriate baudrate

    with SerialInterface(port, baudrate) as serial_interface:
        with open('data.csv', 'w', newline='') as csv_file:
            csv_writer = csv.writer(csv_file)
            csv_writer.writerow(['Timestamp', 'Data'])

            while True:
                data = serial_interface.read()
                timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
                csv_writer.writerow([timestamp, str(data)])
                csv_file.flush()  # Flush to ensure data is written immediately

if __name__ == "__main__":
    main()