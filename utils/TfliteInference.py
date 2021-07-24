import numpy as np
import cv2
import tensorflow as tf
from time import gmtime, strftime
from .detection import get_bounding_boxes


def self_invoke(img):
    TF_MODEL_NAME = "ARC_SDK/src/model.tflite"
    interpreter = tf.lite.Interpreter(model_path=TF_MODEL_NAME)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    scale, zero_point = input_details[0]['quantization']
    out_scale, out_zero_point = output_details[0]['quantization']

    print(out_scale, out_zero_point)

    img = img.astype(np.float32)
    img /= 255
    img = np.int8(img / scale + zero_point)
    img = np.expand_dims(img, axis=0)
    img = np.expand_dims(img, axis=3)

    height = input_details[0]['shape'][1]
    width = input_details[0]['shape'][2]
    print(height, width)

    interpreter.set_tensor(input_details[0]['index'], img)
    interpreter.invoke()
    predictions = interpreter.get_tensor(output_details[0]['index'])
    predictions = np.reshape(predictions, -1)
    img = np.reshape(img, (384, 384))
    s1 = predictions.flatten()
    get_bounding_boxes(img, s1, 1)
