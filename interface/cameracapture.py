import cv2
import base64
import multiprocessing

class CameraCapture(multiprocessing.Process):
	def __init__(self, ws):
		multiprocessing.Process.__init__(self)
		self.size = (200,200)
		self.ws = ws
		self.startCamera()

	def startCamera(self):
		try:
			self.camera = cv2.VideoCapture(0)
			return True
		except:
			print "Camera failed to start"
			return False

	def run(self):
		while True:
			frameData = self.camera.read()[1]
			frameData = cv2.resize(frameData, self.size)
			data = cv2.imencode('.png',frameData)[1]
			self.ws.write_message({'type':"camera", 'data':base64.encodestring(data)})