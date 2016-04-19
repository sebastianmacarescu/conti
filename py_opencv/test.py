import numpy as np
import cv2
import math


cap = cv2.VideoCapture('traseu1.mp4')
paused = False

while(cap.isOpened()):
    
    if cv2.waitKey(20) == ord(' '):
        print "Toggle pause"
        paused = not paused

    if paused:
        continue

    ret, frame = cap.read()

    height, width, channels = frame.shape
    frame = frame[400:height, 0:width]

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.medianBlur(gray, 3)
    ret, gray = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)

    # gray = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 37, 15)
    # cv2.imshow('frame', gray)

    kernel = np.ones((9, 9), np.uint8)
    gray = cv2.morphologyEx(gray, cv2.MORPH_OPEN, kernel)

    edges = cv2.Canny(gray, 100, 200)
    edges = cv2.blur(edges, (5, 5))

    # cv2.imshow('frame', gray)
    # lines = cv2.HoughLines(edges, 1, np.pi/180, 200)

    minLineLength = 100
    maxLineGap = 5
    lines = cv2.HoughLinesP(edges, 1, np.pi/180, 50, minLineLength, maxLineGap)
    if lines is not None:
        # for rho, theta in lines[0]:
        #     a = np.cos(theta)
        #     b = np.sin(theta)
        #     x0 = a*rho
        #     y0 = b*rho
        #     x1 = int(x0 + 1000*(-b))
        #     y1 = int(y0 + 1000*(a))
        #     x2 = int(x0 - 1000*(-b))
        #     y2 = int(y0 - 1000*(a))
        angle = 0.0
        max_angle = -300
        min_angle = 300
        for x1, y1, x2, y2 in lines[0]:
            cv2.line(frame, (x1, y1), (x2, y2), (0, 0, 255), 2)
            local_angle = np.arctan2(y2 - y1, x2 - x1) * 180 / np.pi
            if max_angle < local_angle:
                max_angle = local_angle
            if min_angle > local_angle:
                min_angle = local_angle
            angle += local_angle
        print min_angle, max_angle
        if len(lines[0]) > 0:
            angle = angle / len(lines[0])
            # print angle
            px1 = (int)(width / 2)
            py1 = int(height)
            length = 200

            px2 = (int)(px1 + length * np.sin(angle * np.pi / 180))
            py2 = (int)(py1 + length * np.cos(angle * np.pi / 180))

            py2 -= height
            # print px1, py1, px2, py2
            cv2.line(frame, (px1, py1), (px2, py2), (0, 255, 0), 10)

    gray_c = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
    edges_c = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
    #both = np.hstack((frame, gray_c, edges_c))
    both = np.hstack((frame, edges_c))
    window = cv2.namedWindow('frame', cv2.WINDOW_NORMAL)
    cv2.imshow('frame', both)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()