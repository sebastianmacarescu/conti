import serial
import multiprocessing


class SerialProcess(multiprocessing.Process):

    def __init__(self, input_queue, output_queue, SERIAL_PORT, BAUD_RATE=9600):
        multiprocessing.Process.__init__(self)
        self.input_queue = input_queue
        self.output_queue = output_queue
        self.SERIAL_PORT = SERIAL_PORT
        self.BAUD_RATE = BAUD_RATE
        self.sp = None

    def close(self):
        self.sp.close()

    def writeSerial(self, data):
        try:
            self.sp.write(data.encode())
        except Exception as e:
            print e
            print 'cannot connect'
            # while not self.is_connected():
            #     pass
            # self.writeSerial(data)
        # time.sleep(1)

    def readSerial(self):
        return self.sp.readline().replace("\n", "")

    def run(self):

        self.connect()

        self.sp.flushInput()
        while True:
            # look for incoming tornado request
            if not self.input_queue.empty():
                data = self.input_queue.get()

                # send it to the serial device
                self.writeSerial(data)
                print "writing to serial: " + data

            # look for incoming serial data
            if (self.sp.inWaiting() > 0):
            	data = self.readSerial()
                print "reading from serial: " + data
                # send it back to tornado
            	self.output_queue.put(data)

    def connect(self):
        while not self.is_connected():
            try:
                self.sp = serial.Serial(self.SERIAL_PORT, self.BAUD_RATE, timeout=1)
                print 'Bluetooth connected'
            except Exception as e:
                print e

    def is_connected(self):
        try:
            self.sp.read(0)
            return True
        except:
            return False
