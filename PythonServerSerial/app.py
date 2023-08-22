import csv
import time
import math
from serial_interface import SerialInterface

def main():
    port = "COM8"  # Replace with your actual serial port
    baudrate = 115200  # Replace with the appropriate baudrate

    serial_interface = SerialInterface(port, baudrate)
    with open('data.csv', 'w', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(['Timestamp', 'Data'])

        while True:
            data = serial_interface.read()
            timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
            csv_writer.writerow([timestamp, str(data)])
            csv_file.flush()  # Flush to ensure data is written immediately

def haversine_distance(lat1, lon1, lat2, lon2):
    R = 6371  # Earth's radius in kilometers

    # Convert latitude and longitude from degrees to radians
    lat1, lon1, lat2, lon2 = map(math.radians, [lat1, lon1, lat2, lon2])

    # Haversine formula
    dlat = lat2 - lat1
    dlon = lon2 - lon1
    a = math.sin(dlat / 2) ** 2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2) ** 2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

    distance = R * c  # Distance in kilometers
    return distance

def calculate_speed(lat1, lon1, lat2, lon2, time_in_seconds):
    distance = haversine_distance(lat1, lon1, lat2, lon2)  # Distance in kilometers
    time_in_hours = time_in_seconds / 3600  # Convert time to hours
    speed_kph = distance / time_in_hours
    return speed_kph

def merge_csvs(gps_file, vibration_file, output_file):
    csv_gps = csv.reader(gps_file)
    csv_vibration = csv.reader(vibration_file)
    gps_time_data : list[time.struct_time] = []
    vibration_time_data : list[time.struct_time] = []
    gps_location_data = []
    vibration_data = []
    for line in csv_gps:
        gps_lat = line[0]
        gps_long = line[1]
        gps_time = line[2]
        gps_time_data.append(time.strptime(gps_time, '%Y-%m-%d %H:%M:%S'))
        gps_location_data.append([gps_lat, gps_long])
    
    for line in csv_vibration:
        vibration_time = line[0]
        vibration = line[1]
        vibration_time_data.append(time.strptime(vibration_time, '%Y-%m-%d %H:%M:%S'))
        vibration_data.append(vibration)

    # find the time difference between the first entries of the two files
    gps_min_time = gps_time_data[0]
    vibration_min_time = vibration_time_data[0]
    time_diff = vibration_min_time - gps_min_time
    gps_time_data = [gps_time + time_diff for gps_time in gps_time_data]

    merged_data = []

    # find the closest gps entry for each vibration entry
    for i in range(len(vibration_time_data)):
        vibration_time = vibration_time_data[i]
        min_diff = abs((gps_time_data[0] - vibration_time).seconds)
        min_j = 0
        # find gps entry with closest time
        for j in range(len(gps_time_data)):
            diff = abs((gps_time_data[j] - vibration_time).seconds)
            if diff < min_diff:
                min_diff = diff
                min_j = j

        lat = gps_location_data[min_j][0]
        long = gps_location_data[min_j][1]
        # calculate speed using location
        speed = 0
        if len(merged_data) != 0:
            old_lat = merged_data[-1][2]
            old_long = merged_data[-1][3]
            old_time = merged_data[-1][1]
            speed = calculate_speed(old_lat, old_long, lat, long, (vibration_time-old_time).seconds)

        merged_data.append([vibration_time, vibration_data[i], lat, long, speed])
    
    # write merged data to file
    with open(output_file, 'w', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)
        csv_writer.writerow(['Timestamp', 'Vibration', 'Latitude', 'Longitude', 'Velocity (km/h)'])
        for line in merged_data:
            csv_writer.writerow(line)


        

if __name__ == "__main__":
    main()