from utils.detection import get_bounding_boxes
from utils.TfliteInference import self_invoke
from time import gmtime, strftime
from PIL import Image, ImageTk
import serial
import numpy as np
import tkinter as tk
import cv2
import argparse
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

# program parameters
IMG_SIZE = (192, 192)
PREDICTION_LEN = 24 * 24 * 24  # grid_len * grid_len * yolo
parser = argparse.ArgumentParser()
parser.add_argument("-p", "--port", help="specify the port of WE-1 Plus",
                    dest="port", default="/dev/ttyUSB0")
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
BAUD_RATE = int(args.baud)
THRESH = float(args.thresh)
IOU_THRESH = float(args.iou_thresh)

# open serial port
try:
    ser = serial.Serial(PORT, BAUD_RATE)
except serial.serialutil.SerialException:
    print("[Error] Specified port device not found\nAbort program\n")
    exit(-1)

has_defect = 0
is_paused = 0
frame_count = 0
status_str = ''
frame_count_str = ''
detect_result_str = ''
img_RGB = np.array([0])


def pause_handler():
    global is_paused
    global status_str
    is_paused = not is_paused
    if is_paused:
        status_str = "Image capture paused!"
        pause_btn['text'] = 'Resume'
    else:
        status_str = 'Image capture resumed'
        pause_btn['text'] = 'Pause'
    str_label3['text'] = status_str


def save_handler():
    global img_RGB
    global frame_count
    global status_str
    current_time = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
    img_name = "detection_{}_{}.png".format(frame_count, current_time)
    cv2.imwrite(img_name, img_RGB)
    status_str = "Image Saved successfully"
    str_label3['text'] = status_str
    print("{}\nwritten!".format(img_name))


def exit_handler():
    exit(0)


# gui parameters
pad = 5
align_mode = 'nswe'

window = tk.Tk()
window.title("Solder Joint Inspection")
window.geometry("600x400")
window.configure(bg='floral white')
window.resizable(0, 0)  # not resizable

captured_frame = tk.Label(
    window, width=IMG_SIZE[0], height=IMG_SIZE[1], bg='white')
message_frame = tk.Frame(
    window, width=IMG_SIZE[0]/2, height=IMG_SIZE[1]/2, bg='white')
operate_frame = tk.Frame(
    window, width=IMG_SIZE[0]/2, height=IMG_SIZE[1]/2, bg='floral white')

captured_frame.grid(column=0, row=0, padx=pad, pady=pad,
                    rowspan=2, sticky=align_mode)
message_frame.grid(column=1, row=0, padx=pad, pady=pad, sticky=align_mode)
operate_frame.grid(column=1, row=1, padx=pad, pady=pad, sticky=align_mode)


ser.reset_input_buffer()  # clear input buffer

# message frame setup
str_label1 = tk.Label(
    message_frame, text=frame_count_str, width=23, height=6, bg='white')
str_label2 = tk.Label(
    message_frame, text=detect_result_str, width=23, height=6, bg='white')
str_label2.config(font=("Courier", 10, 'bold'))
str_label3 = tk.Label(
    message_frame, text=status_str, width=23, height=6, bg='white')
str_label1.grid(column=0, row=0)
str_label2.grid(column=0, row=1)
str_label3.grid(column=0, row=2)

# operate frame setup
pause_btn = tk.Button(
    operate_frame, text="Resume" if is_paused else "Pause", width=20)
save_btn = tk.Button(operate_frame, text="Save Image", width=20)
exit_btn = tk.Button(operate_frame, text="Exit")
pause_btn.grid(column=0, row=0)
save_btn.grid(column=0, row=1)
exit_btn.grid(column=0, row=2)
pause_btn['command'] = pause_handler
save_btn['command'] = save_handler
exit_btn['command'] = exit_handler


def capture_stream():
    img_data = []
    predictions = []
    start_signal_count = 0

    global has_defect
    global frame_count
    global img_RGB
    global is_paused
    global status_str
    # transfer image (traferred in byte type)
    if not is_paused:
        while ser.inWaiting and start_signal_count != 10:
            data = ser.read()
            if data == b'7':
                start_signal_count += 1
            else:
                start_signal_count = 0
        start_signal_count = 0
        while ser.inWaiting:
            data = ser.read(IMG_SIZE[0])
            img_data.append(list(data))
            if len(img_data) == IMG_SIZE[1]:
                frame_count += 1
                data_array = np.array(img_data, dtype=np.uint8)

                # image upscaling
                data_array = np.repeat(data_array, repeats=2, axis=0)
                data_array = np.repeat(data_array, repeats=2, axis=1)
                print("image transfer complete!")

                # transfer predictions (transferred in int type)
                byte_queue = []
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
                img_RGB, has_defect = get_bounding_boxes(data_array, predictions,
                                                         frame_count, THRESH, IOU_THRESH)
                break

    # message frame setup
    frame_count_str = "Frame no.{}".format(frame_count)
    if has_defect:
        detect_result_str = "Defect joint detected!"
        str_label2['fg'] = 'IndianRed2'
    else:
        detect_result_str = "Board QC test passed!"
        str_label2['fg'] = 'SeaGreen2'
    str_label1['text'] = frame_count_str
    str_label2['text'] = detect_result_str

    # image showing setup
    img_pil = Image.fromarray(img_RGB)
    img_tk = ImageTk.PhotoImage(image=img_pil)
    captured_frame.configure(image=img_tk)
    captured_frame.image = img_tk

    window.after(10 if not is_paused else 1000, capture_stream)


capture_stream()
window.mainloop()
cv2.destroyAllWindows()
