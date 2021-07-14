import numpy as np
import cv2

# get the bounding box of detecting image and show
# arguments:
# img:          ndarray, 2d array of detecting image
# predictions:  list, 1d list of uint8 predictions from yolo output
# return: none


def get_bounding_boxes(img, predictions):
    cv2.imshow('Detecting result', img)
    cv2.waitKey(1)
    return
