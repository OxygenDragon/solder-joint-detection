from utils.detection import get_bounding_boxes
from utils.TfliteInference import self_invoke
import serial
import numpy as np
import cv2
import argparse
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

IMG_SIZE = (384, 384)
PREDICTION_LEN = 24 * 24 * 24  # grid_len * grid_len * yolo

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help="specify the port of WE-1 Plus",
                    dest="port", default="ttyUSB0")
parser.add_argument("-b", "--baud", help="specify the baud rate of transferring\
                     image and predictions",
                    dest="baud", default=921600)
parser.add_argument("-t", "--thresh", help="specify the thresh of detecting",
                    dest="thresh", default=0.15)
parser.add_argument("-i", "--iou_thresh", help="specify the iou thresh of\
                    detecting",
                    dest="iou_thresh", default=0.1)
args = parser.parse_args()


PORT = args.port
BAUD_RATE = args.baud
THRESH = args.thresh
IOU_THRESH = args.iou_thresh

print("\nArguments used:\nPort: {},\t\tBaud rate: {}".format(
    PORT, BAUD_RATE))
print("Conf thresh: {},\tIOU thresh: {}\n".format(THRESH, IOU_THRESH))


def main():
    try:
        ser = serial.Serial(PORT, BAUD_RATE)
    except serial.serialutil.SerialException:
        print("[Error] Specified port device not found\nAbort program\n")
        exit(-1)

    img_data = []
    predicitons = []
    start_signal_count = 0
    count = 0

    try:
        ser.reset_input_buffer()
        # cv2 window setting up
        print("pending start signal...")
        while True:
            print(ser.inWaiting())
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
                    # waiting for prediction start signal
                    while ser.inWaiting and start_signal_count != 10:
                        data = ser.read()
                        if data == b'8':
                            start_signal_count += 1
                        else:
                            start_signal_count = 0
                    start_signal_count = 0
                    while ser.inWaiting:
                        current_line = ser.readline().strip()
                        value = current_line.decode('ascii')
                        byte_queue.append(value)
                        if (len(byte_queue) == PREDICTION_LEN):
                            break
                    predictions = np.array(byte_queue, dtype=np.int8)
                    img_RGB, result = get_bounding_boxes(data_array, predictions, count, THRESH,
                                                         IOU_THRESH)
                    cv2.imshow('Detecting result', img_RGB)
                    key = cv2.waitKey(1)
                    # self_invoke(data_array)
                    print("draw bouding box complete!")
                    img_data = []
                    break

    except KeyboardInterrupt:
        ser.close()


if __name__ == '__main__':
    main()
