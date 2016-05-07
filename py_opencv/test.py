import numpy as np
import cv2
import math
import json
import socket
import capture


class Line:
    def __init__(self, x1, y1, x2, y2):
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2
        self.length = math.sqrt(math.pow(x2 - x1, 2) + math.pow(y2 - y1, 2))
        self.angle = np.arctan2(y1 - y2, x2 - x1) * 180 / np.pi

    def __str__(self):
        return json.dumps(self.__dict__)


paused = False
font = cv2.FONT_HERSHEY_SIMPLEX

cap = capture.FileCapture('hol.mp4')
# cap = capture.FileCapture('traseu2.mp4')
# cap = capture.SocketCapture('192.168.88.21')
# cap = capture.SocketCapture('192.168.1.2')
cap.open()

while(cap.isOpened()):

    if cv2.waitKey(10) == ord(' '):
        print "Toggle pause"
        paused = not paused

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    if paused:
        continue

    frame = cap.readFrame()
    if frame is None:
        break


    window = cv2.namedWindow('frame', cv2.WINDOW_NORMAL)
    # cv2.imshow('frame', frame)

    height, width, channels = frame.shape
    # frame = frame[400:height, 0:width]

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.medianBlur(gray, 3)
    ret, gray = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)

    # gray = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 37, 15)
    # cv2.imshow('frame', gray)

    kernel = np.ones((9, 9), np.uint8)
    se1 = cv2.getStructuringElement(cv2.MORPH_RECT, (30,30))
    gray = cv2.morphologyEx(gray, cv2.MORPH_OPEN, se1)
    gray = cv2.morphologyEx(gray, cv2.MORPH_OPEN, se1)

    edges = cv2.Canny(gray, 100, 200)
    edges = cv2.blur(edges, (5, 5))

    # cv2.imshow('frame', gray)
    # lines = cv2.HoughLines(edges, 1, np.pi/180, 200)

    minLineLength = 100
    maxLineGap = 5
    lines = cv2.HoughLinesP(edges, 1, np.pi/180, 50, minLineLength, maxLineGap)

    my_lines = []

    if lines is not None:

        for x1, y1, x2, y2 in lines[0]:
            cv2.line(frame, (x1, y1), (x2, y2), (0, 0, 255), 2)
            line = Line(x1, y1, x2, y2)
            my_lines.append(line)
        
        lines_by_angle = {angle: [] for angle in range(-90, 90, 3)}

        for line in my_lines:
            if int(line.angle) in lines_by_angle:
                new_angle = int(line.angle)
            elif int(line.angle) - 1 in lines_by_angle:
                new_angle = int(line.angle) - 1
            elif int(line.angle) + 1 in lines_by_angle:
                new_angle = int(line.angle) + 1
            lines_by_angle[new_angle].append(line)

        idx = 0
        colors = [
            (255, 0, 0),
            (0, 255, 0),
            (0, 0, 255)
        ]
        min_len = 200
        final_lines = []
        for angle in lines_by_angle:
            lines_by_angle[angle] = sorted(lines_by_angle[angle], key=lambda k: (-k.length, min(k.x1, k.x2)))
            if len(lines_by_angle[angle]) > 0 and lines_by_angle[angle][0].length >= min_len:
                best_line = lines_by_angle[angle][0]
                # print best_line
                # cv2.line(frame, (best_line.x1, best_line.y1), (best_line.x2, best_line.y2), colors[idx % 3], 5)
                idx += 1
                final_lines.append(best_line)

        avg_angle_sum = 0
        avg_angle = 0
        for line in final_lines:
            avg_angle_sum += line.angle
        if len(final_lines) > 0:
            avg_angle = avg_angle_sum / len(final_lines)
        else:
            avg_angle = 90
        print avg_angle

        px1 = (int)(width / 2)
        py1 = int(height)
        length = 300

        px2 = (int)(px1 + length * np.sin(avg_angle * np.pi / 180))
        py2 = (int)(py1 + length * np.cos(avg_angle * np.pi / 180))

        py2 -= height
        # cv2.line(frame, (px1, py1), (px2, py2), (0, 255, 0), 10)
        # cv2.putText(frame, str(avg_angle), (10,500), font, 3, (255, 255, 255), 2)

        # for i in range(0, 2):
        #     line = my_lines[i]
        #     cv2.line(frame, (line.x1, line.y1), (line.x2, line.y2), (0, 255 * (i % 2), 255), 5)
        
        # print my_lines[0]
        # if len(lines[0]) > 0:
        #     angle = angle / len(lines[0])
        #     # print angle
        #     px1 = (int)(width / 2)
        #     py1 = int(height)
        #     length = 200

        #     px2 = (int)(px1 + length * np.sin(angle * np.pi / 180))
        #     py2 = (int)(py1 + length * np.cos(angle * np.pi / 180))

        #     py2 -= height
        #     # print px1, py1, px2, py2
        #     cv2.line(frame, (px1, py1), (px2, py2), (0, 255, 0), 10)

    gray_c = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
    edges_c = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
    both = np.hstack((frame, gray_c, edges_c))
    # both = np.hstack((frame, edges_c))
    window = cv2.namedWindow('frame', cv2.WINDOW_NORMAL)
    cv2.imshow('frame', both)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.close()
cv2.destroyAllWindows()