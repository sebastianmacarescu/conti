import socket
import threading
from Queue import *


def recvall(sock, count):
    buf = b''
    try:
        # while count:
        #     newbuf = sock.recv(count)
        #     print newbuf
        #     if not newbuf:
        #         return buf
        #     buf += newbuf
        #     count -= len(newbuf)
        buf = sock.recv(count)
    except:
        return None
    return buf


class QueuePipe:

    def __init__(self, send_q=Queue(), recv_q=Queue()):
        self.send_q = send_q
        self.recv_q = recv_q


class ClientSocket(threading.Thread):

    def __init__(self, IP, PORT, queuePipe=QueuePipe()):
        threading.Thread.__init__(self)
        self.IP = IP
        self.PORT = PORT
        self.opened = False
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.queuePipe = queuePipe

    def open(self):
        if self.opened:
            return
        try:
            self.socket.settimeout(0.5)
            self.socket.connect((self.IP, int(self.PORT)))
            self.opened = True
            print "Socket connected on %s:%s" % (self.IP, self.PORT)
        except Exception as e:
            self.opened = False
            print e

    def isOpened(self):
        return self.opened

    def close(self):
        self.socket.close()

    def read(self):
        if not self.recv_q().empty():
            return self.recv_q().get()
        return None

    def write(self, data):
        self.send_q().put(data)

    def send_q(self):
        return self.queuePipe.send_q

    def recv_q(self):
        return self.queuePipe.recv_q

    def run(self):
        while True:
            if not self.isOpened():
                continue
            data = recvall(self.socket, int(4096))
            if data:
                self.recv_q().put(data)

            while not self.send_q().empty():
                to_send = self.send_q().get()
                to_send += "\n"
                self.socket.sendall(to_send)
