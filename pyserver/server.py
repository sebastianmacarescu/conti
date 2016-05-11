import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.gen
from tornado.options import define, options
import os
import multiprocessing
import serialworker
import cameracapture
import json

from py_opencv import capture
from py_opencv import frame_process


# import multiprocessing, logging
# logger = multiprocessing.log_to_stderr()
# logger.setLevel(multiprocessing.SUBDEBUG)

define("port", default=8080, help="run on the given port", type=int)
define("serial", default="COM10", help="serial port for bluetooth", type=str)

CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
STATIC_ROOT = os.path.join(CURRENT_DIR, 'static')

clients = []

input_queue = multiprocessing.Queue()
output_queue = multiprocessing.Queue()

video_cap = capture.SocketCapture('192.168.88.21')
# video_cap = capture.FileCapture('py_opencv\\traseu1.mp4')


class IndexHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('index.html')


class StaticFileHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('main.js')


class WebSocketHandler(tornado.websocket.WebSocketHandler):
    def open(self):
        print 'new connection'
        clients.append(self)
        self.write_message({'type': "info", 'data': "connected"})
        # self.cameraCapture = cameracapture.CameraCapture(self)
        # self.cameraCapture.daemon = True
        # self.cameraCapture.start()

    def on_message(self, message):
        print 'tornado received from client: %s' % json.dumps(message)
        # self.write_message('ack')
        input_queue.put(message)

    def on_close(self):
        print 'connection closed'
        clients.remove(self)
        # self.cameraCapture.stop()


# check the queue for pending messages, and rely that to all connected clients
def checkQueue():
    if not output_queue.empty():
        message = output_queue.get()
        for c in clients:
            c.write_message(message)


def checkVideoCapture():
    if not video_cap.isOpened():
        video_cap.open()

    frame = video_cap.readFrame()
    if frame is None:
        return

    data = frame_process.process_frame(frame)
    print data["total_avg_angle"]
    b64 = capture.b64(data['frame'])
    # b64 = video_cap.b64()
    for c in clients:
        c.write_message({
            'type': "camera",
            'data': b64
        })
    input_queue.put("camera %s;" % data["total_avg_angle"])

if __name__ == '__main__':
    tornado.options.parse_command_line()
    app = tornado.web.Application(
        handlers=[
            (r"/", IndexHandler),
            (r'/static/(.*)', tornado.web.StaticFileHandler, {'path': STATIC_ROOT}),
            (r"/ws", WebSocketHandler)
        ]
    )
    httpServer = tornado.httpserver.HTTPServer(app)
    httpServer.listen(options.port)
    print "Listening on port:", options.port

    mainLoop = tornado.ioloop.IOLoop.instance()
    # adjust the scheduler_interval according to the frames sent by the serial port
    scheduler = tornado.ioloop.PeriodicCallback(checkQueue, 100, io_loop=mainLoop)
    camera_scheduler = tornado.ioloop.PeriodicCallback(checkVideoCapture, 10, io_loop=mainLoop)

    sp = serialworker.SerialProcess(input_queue, output_queue, options.serial)
    sp.daemon = True

    sp.start()
    scheduler.start()
    camera_scheduler.start()
    mainLoop.start()
