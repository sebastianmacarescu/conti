import cv2
import socket
import numpy as np


def recvall(sock, count):
    buf = b''
    while count:
        newbuf = sock.recv(count)
        if not newbuf: return None
        buf += newbuf
        count -= len(newbuf)
    return buf


class FileCapture:

    def __init__(self, file_name):
        self.file_name = file_name
        self.cap = cv2.VideoCapture(file_name)

    def open(self):
        pass

    def isOpened(self):
        return self.cap.isOpened()

    def readFrame(self):
        ret, frame = self.cap.read()
        return frame

    def close(self):
        self.cap.release()


class SocketCapture(FileCapture):
    def __init__(self, ip):
        self.ip = ip
        self.camera_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.opened = False

    def open(self):
        self.camera_socket.connect((self.ip, 8080))
        self.opened = True

    def isOpened(self):
        return self.opened

    def readFrame(self):
        length = recvall(self.camera_socket, 6)
        if not length:
            return None
        data = recvall(self.camera_socket, int(length))
        if not data:
            return None

        nparr = np.asarray(bytearray(data), dtype=np.uint8)
        frame = cv2.imdecode(nparr, -10) # for 4 chanels (with alpha)

        return frame

    def close(self):
        self.camera_socket.close()
