import serial
import numpy as np
import cv2
from utils.detection import get_bounding_boxes

PORT = '/dev/ttyUSB0'
BAUD_RATE = 921600
IMG_SIZE = (384, 384)
PREDICTION_LEN = 24 * 24 * 24  # grid_len * grid_len * yolo
ser = serial.Serial(PORT, BAUD_RATE)

img_data = []
predicitons = []
start_signal_count = 0
count = 0
try:
    ser.reset_input_buffer()
    print("pending start signal...")
    while True:
        # waiting for image start signal
        while ser.inWaiting and start_signal_count != 10:
            data = ser.read()
            if data == b'7':
                start_signal_count += 1
            else:
                start_signal_count = 0
        print("start transferring image")
        start_signal_count = 0
        while ser.inWaiting:
            data = ser.read(IMG_SIZE[0])
            img_data.append(list(data))
            if len(img_data) == IMG_SIZE[1]:
                count += 1
                data_array = np.array(img_data, dtype=np.uint8)
                print("image transfer complete!")
                print("image size", data_array.shape)

                # transferring predictions
                byte_queue = []
                byte_queue_count = 0
                stop_point = PREDICTION_LEN / 576
                # waiting for prediction start signal
                while ser.inWaiting and start_signal_count != 10:
                    data = ser.read()
                    print(data)
                    if data == b'8':
                        start_signal_count += 1
                    else:
                        start_signal_count = 0
                start_signal_count = 0
                while ser.inWaiting and byte_queue_count < stop_point:
                    byte_queue += list(ser.read(576))
                    byte_queue_count += 1
                for i in range(PREDICTION_LEN):
                    byte_queue[i] = int(byte_queue[i])
                predictions = np.array(byte_queue, dtype=np.int8)
                f = open("prediction.txt", 'w')
                for x in range(PREDICTION_LEN):
                    f.write(str(predictions[x]) + ', ')
                f.close()
                get_bounding_boxes(data_array, predictions, count)
                print("draw bouding box complete!")
                img_data = []
                break

except KeyboardInterrupt:
    ser.close()
