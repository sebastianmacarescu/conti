import cv2
import socket
import numpy as np
import base64


def recvall(sock, count):
    buf = b''
    while count:
        newbuf = sock.recv(count)
        if not newbuf:
            return None
        buf += newbuf
        count -= len(newbuf)
    return buf


def b64(frame):
    if frame is not None:
        return base64.encodestring(cv2.imencode('.png', frame)[1])


class FileCapture:

    def __init__(self, file_name):
        self.file_name = file_name
        self.cap = cv2.VideoCapture(file_name)
        self.last_frame = None

    def open(self):
        pass

    def isOpened(self):
        return self.cap.isOpened()

    def readFrame(self):
        if not self.isOpened():
            return None
        ret, frame = self.cap.read()
        self.last_frame = frame
        return frame

    def b64(self):
        if self.last_frame is not None:
            return base64.encodestring(cv2.imencode('.png', self.last_frame)[1])

    def close(self):
        self.cap.release()


class SocketCapture(FileCapture):
    def __init__(self, ip):
        self.ip = ip
        self.camera_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.opened = False
        self.last_frame = None

    def open(self):
        self.camera_socket.connect((self.ip, 8080))
        self.opened = True

    def isOpened(self):
        return self.opened

    def readFrame(self):
        if not self.isOpened():
            return None

        length = recvall(self.camera_socket, 6)
        if not length:
            return None
        data = recvall(self.camera_socket, int(length))
        if not data:
            return None

        nparr = np.asarray(bytearray(data), dtype=np.uint8)
        frame = cv2.imdecode(nparr, -10) # for 4 chanels (with alpha)
        self.last_frame = frame
        return frame

    def close(self):
        self.camera_socket.close()
