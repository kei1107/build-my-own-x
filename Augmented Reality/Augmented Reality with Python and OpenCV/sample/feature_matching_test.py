import cv2

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
    # draw first 15 matches.
    cap = cv2.drawMatches(model,kp_model,cap,kp_frame,matches[:MIN_MATCHES],0,flags=2)
    # show result
    cv2.imshow("frame",cap)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
else:
    print("Not enough matches have been found - %d/%d" % (len(matches),MIN_MATCHES))
