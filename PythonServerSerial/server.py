from serial import Serial

ser = Serial('COM6', 115200)

while True:
    line = ser.readline().decode().rstrip()
    print(line)


ser.close()