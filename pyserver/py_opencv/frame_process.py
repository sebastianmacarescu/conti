import cv2
import math
import json
import numpy as np


class Line:
    def __init__(self, x1, y1, x2, y2):
        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2
        self.length = math.sqrt(math.pow(x2 - x1, 2) + math.pow(y2 - y1, 2))
        self.angle = np.arctan2(y2 - y1, x2 - x1) * 180 / np.pi

        #make angle relative to y axis
        if self.angle >= 0:
            self.angle = 90 - self.angle
        else:
            self.angle = -90 - self.angle

    def __str__(self):
        return json.dumps(self.__dict__)


def distance(x1, y1, x2, y2):
    return math.sqrt((x1 - x2)**2 + (y1 - y2)**2)


def line_min_dist(a, b):
    dist = [
        distance(a.x1, a.y1, b.x1, b.y1),
        distance(a.x1, a.y1, b.x2, b.y2),
        distance(a.x2, a.y2, b.x1, b.y1),
        distance(a.x2, a.y2, b.x2, b.y2)
    ]
    return min(dist)


def half(a, b):
    return (a + b) / 2


def process_frame(frame):
    height, width, channels = frame.shape
    # frame = frame[400:height, 0:width]
    # height = height - 400

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
    total_avg_angle = 0
    if lines is not None:
        first = True
        for x1, y1, x2, y2 in lines[0]:
            line = Line(x1, y1, x2, y2)
            my_lines.append(line)
            # if first is True:
            cv2.line(frame, (x1, y1), (x2, y2), (0, 0, 255), 2)
                # first = False
                # print line.angle


        rows = 6
        int_w = height / rows
        total_avg_sum = 0
        for row in range(1, rows + 1):

            avg_angle_sum = 0
            avg_angle = 0
            weights = []
            for line in my_lines:
                if line.y1 < (row - 1) * int_w or line.y1 > row * int_w: continue
                avg_angle_sum += line.angle * (line.length / 1000)
                if line.length / 1000 not in weights:
                    weights.append(line.length / 1000)

            if len(weights) > 0:
                avg_angle = avg_angle_sum / sum(weights)
            else:
                avg_angle = 0
            # avg_angle = -avg_angle
            # print avg_angle
            total_avg_sum += avg_angle * (rows - row + 1)

            length = -(int)(10 * avg_angle)
            px1 = (int)(width / 2)
            py1 = (int)((2 * row - 1) * int_w / 2)

            px2 = px1 + length
            py2 = py1

            # px2 = (int)(px1 + length * np.sin(avg_angle * np.pi / 180))
            # py2 = (int)(py1 + length * np.cos(avg_angle * np.pi / 180))

            cv2.line(frame, (px1, py1), (px2, py2), (0, 255, 0), 5)
        
        total_avg_angle = total_avg_sum / ( rows * (rows + 2) / 2 )
        print total_avg_angle
        px1 = (int)(width / 2)
        py1 = (int)(height / 2)
        px2 = px1 - (int)(10 * total_avg_angle)
        py2 = py1
        cv2.line(frame, (px1, py1), (px2, py2), (0, 0, 255), 10)

    return {
        'frame': frame,
        'total_avg_angle': total_avg_angle
    }
