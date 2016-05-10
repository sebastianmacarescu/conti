"""
---------------ALG1---------------------------
----------------------------------------------
"""

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


"""
---------------ALG2---------------------------
----------------------------------------------
"""

def merge_lines(my_lines):
    merged_lines = []
    merged_indexes = {}
    for _i in range(len(my_lines)):
        if merged_indexes.get(_i, False) is True: continue
        for _j in range(len(my_lines)):
            if merged_indexes.get(_j, False) is True: continue
            
            if _i == _j: continue

            la = my_lines[_i]
            lb = my_lines[_j]
            if line_min_dist(la, lb) < 10 and abs(la.angle - lb.angle) < 5:
                merged = Line(
                    half(la.x1, la.x2),
                    min([la.y1, la.y2, lb.y1, lb.y2]),
                    half(lb.x1, lb.x2),
                    max([la.y1, la.y2, lb.y1, lb.y2])
                )
                merged_lines.append(merged)
                merged_indexes[_i] = True
                merged_indexes[_j] = True
                break
    print "Before %s merged %s" %(len(my_lines), len(merged_lines))
    return merged_lines

#Merge lines
        merged_lines = merge_lines(my_lines)
        
        for a in merged_lines:
            cv2.line(merged_img, (a.x1, a.y1), (a.x2, a.y2), (0, 0, 255), 2)