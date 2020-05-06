import cv2
import numpy as np

MIN_MATCHES = 15

cap = cv2.imread("scene.jpg", 0)
model = cv2.imread("model.jpg", 0)

# Initiate ORB detector
orb = cv2.ORB_create()
# create brute force matcher object
bf = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=True)
# Compute model keypoints and its descriptors
kp_model, des_model = orb.detectAndCompute(model, None)
# Compute scene keypoints and its descriptors
kp_frame, des_frame = orb.detectAndCompute(cap, None)
# Match frame descriptors with model descriptors
matches = bf.match(des_model, des_frame)
# Sort them in the order of their distance
matches = sorted(matches, key=lambda x: x.distance)

if len(matches) > MIN_MATCHES:
    # differenciate between source points and destination points
    src_pts = np.float32([kp_model[m.queryIdx].pt for m in matches]).reshape(-1,1,2)
    dst_pts = np.float32([kp_frame[m.trainIdx].pt for m in matches]).reshape(-1, 1, 2)
    # compute Homography
    M, mask = cv2.findHomography(src_pts, dst_pts, cv2.RANSAC, 5.0)

    # Draw a rectangle that marks the found model in the frame
    h, w = model.shape
    pts = np.float32([[0, 0], [0, h - 1], [w - 1, h - 1], [w - 1, 0]]).reshape(-1, 1, 2)
    # project corners into frame
    dst = cv2.perspectiveTransform(pts, M)
    # connect them with lines
    cap = cv2.polylines(cap, [np.int32(dst)], True, 255, 3, cv2.LINE_AA)

    # show result
    cv2.imshow("frame",cap)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
else:
    print("Not enough matches have been found - %d/%d" % (len(matches),MIN_MATCHES))

