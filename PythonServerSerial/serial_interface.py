from serial import Serial

class SerialInterface():
    def __init__(self, port, baudrate):
        self.ser = Serial(port, baudrate)

    def read(self):
        line = self.ser.readline().decode().rstrip()
        # if the line is a comment, read the next line
        if line.startswith("//"):
            return self.read()
        segments = line.split(',')
        key_value_pairs = [(segment.split(':')[0], int(segment.split(':')[1])) for segment in segments]
        return dict(key_value_pairs)

    def close(self):
        self.ser.close()