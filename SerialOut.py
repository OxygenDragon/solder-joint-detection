import serial
import numpy as np
import cv2
from utils.detection import get_bounding_boxes

PORT = '/dev/ttyUSB0'
BAUD_RATE = 921600
ser = serial.Serial(PORT, BAUD_RATE)

IMG_SIZE = (416, 416)
PREDICTION_LEN = 26 * 26 * 33
img_data = []
predicitons = []
start_signal_count = 0
count = 0
try:
    print("pending start signal...")
    while True:
        while ser.inWaiting and start_signal_count != 10:
            data = ser.read()
            print(data)
            if data == b'7':
                start_signal_count += 1
            else:
                start_signal_count = 0
        print("start transferring image")
        start_signal_count = 0
        while ser.inWaiting:
            data = ser.read(IMG_SIZE[0])
            img_data.append(list(data))
            count += 1
            if len(img_data) == IMG_SIZE[1]:
                data_array = np.array(img_data, dtype=np.uint8)
                print("image transfer complete!")
                print("image size", data_array.shape)
                predicitons = np.empty(1, dtype=np.uint8)
                while ser.inWaiting:
                    data = ser.read(PREDICTION_LEN)
                    predictions = np.array(list(data), dtype=np.uint8)
                    break
                get_bounding_boxes(data_array, predictions)
                cv2.imshow('result', data_array)
                cv2.waitKey(1)
                img_data = []
                count = 0
                break

except KeyboardInterrupt:
    ser.close()
