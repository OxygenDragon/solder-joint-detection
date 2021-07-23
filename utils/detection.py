import numpy as np
import cv2
import tensorflow as tf
from time import gmtime, strftime


class_name = [
    "poor",
    "short",
    "excess"
]
out_zero_point = -23
out_scale = 0.024364763870835304
confidence_thresh = 0.1


# get the bounding box of detecting image and show
# arguments:
# img:          ndarray, 2d array of detecting image
# predictions:  list, 1d list of int8 predictions from yolo output
# return: none


def get_bounding_boxes(img, predictions, img_number):
    # To draw RGB bounding boxes
    img_RGB = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    predictions = np.reshape(predictions, (1, 24, 24, 24))
    predictions = np.float32((predictions - out_zero_point) * out_scale)
    predictions = _detection_layer(predictions, num_classes=3, anchors=[(
        10, 14),  (23, 27),  (37, 58), ], img_size=[384, 384], data_format='NHWC')
    boxes, classes, scores = handle_predictions(
        predictions, confidence=confidence_thresh, iou_threshold=0.0)
    draw_boxes(boxes, classes, scores, img_RGB, class_name)
    cv2.imshow('Detecting result', img_RGB)
    key = cv2.waitKey(1)
    image_window_handler(key, img_RGB, img_number)


def image_window_handler(key, frame, img_number):
    if key % 256 == 27:
        # ESC pressed
        print("Escape hit, closing...")
        cv2.destroyAllWindows()
        exit(0)
    elif key % 256 == 32:
        # SPACE pressed
        current_time = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
        img_name = "detection_{}_{}.png".format(img_number, current_time)
        cv2.imwrite(img_name, frame)
        print("{} written!".format(img_name))


def handle_predictions(predictions, confidence=0.6, iou_threshold=0.5):
    boxes = predictions[:, :, :4]
    box_confidences = np.expand_dims(predictions[:, :, 4], -1)
    box_confidences = np.maximum(box_confidences, 0)
    box_confidences = np.minimum(box_confidences, 1)

    box_class_probs = predictions[:, :, 5:]
    box_class_probs = np.maximum(box_class_probs, 0)
    box_class_probs = np.minimum(box_class_probs, 1)

    box_scores = list(box_confidences * box_class_probs)
    box_class_scores = np.max(box_scores, axis=-1)
    box_classes = np.argmax(box_scores, axis=-1)
    pos = np.where(box_class_scores >= confidence)
    boxes = np.array(boxes)
    boxes = boxes[pos]
    classes = box_classes[pos]
    scores = box_class_scores[pos]
    n_boxes, n_classes, n_scores = nms_boxes(
        boxes, classes, scores, iou_threshold)

    if n_boxes:
        boxes = np.concatenate(n_boxes)
        classes = np.concatenate(n_classes)
        scores = np.concatenate(n_scores)
        return boxes, classes, scores

    else:
        return None, None, None


def nms_boxes(boxes, classes, scores, iou_threshold):
    nboxes, nclasses, nscores = [], [], []
    for c in set(classes):
        inds = np.where(classes == c)
        b = boxes[inds]
        c = classes[inds]
        s = scores[inds]

        x = b[:, 0]
        y = b[:, 1]
        w = b[:, 2]
        h = b[:, 3]

        areas = w * h
        order = s.argsort()[::-1]

        keep = []
        while order.size > 0:
            i = order[0]
            keep.append(i)

            xx1 = np.maximum(x[i], x[order[1:]])
            yy1 = np.maximum(y[i], y[order[1:]])
            xx2 = np.minimum(x[i] + w[i], x[order[1:]] + w[order[1:]])
            yy2 = np.minimum(y[i] + h[i], y[order[1:]] + h[order[1:]])

            w1 = np.maximum(0.0, xx2 - xx1 + 1)
            h1 = np.maximum(0.0, yy2 - yy1 + 1)

            inter = w1 * h1
            ovr = inter / (areas[i] + areas[order[1:]] - inter)
            inds = np.where(ovr <= iou_threshold)[0]
            order = order[inds + 1]

        keep = np.array(keep)

        nboxes.append(b[keep])
        nclasses.append(c[keep])
        nscores.append(s[keep])
    return nboxes, nclasses, nscores


def draw_boxes(boxes, classes, scores, img, class_name):
    if boxes is not None:
        for box, score, cls in zip(boxes, scores, classes):
            left = (int)(box[0] - box[2]/2)
            top = (int)(box[1] - box[3]/2)
            right = (int)(box[0] + box[2]/2)
            bottom = (int)(box[1] + box[3]/2)
            color = [
                (164, 145, 250),
                (63, 212, 162),
                (232, 209, 77),
            ]
            thick = 2
            cv2.putText(
                img,
                '{}: {:.2f}%'.format(class_name[cls], score * 100),
                (left, bottom + 20) if top < 30 else (left, top - 10), 3,
                thick / 3.5, color[cls], thickness=1
            )
            cv2.rectangle(img, (left, top), (right, bottom), color[cls], thick)


def _get_size(shape, data_format):
    if len(shape) == 4:
        shape = shape[1:]
    return shape[1:3] if data_format == 'NCHW' else shape[0:2]


def _detection_layer(predictions, num_classes, anchors, img_size, data_format):
    num_anchors = len(anchors)

    shape = np.array(predictions).shape
    grid_size = _get_size(shape, data_format)
    dim = grid_size[0] * grid_size[1]
    bbox_attrs = 5 + num_classes

    if data_format == 'NCHW':
        predictions = tf.reshape(
            predictions, [-1, num_anchors * bbox_attrs, dim])
        predictions = tf.transpose(predictions, [0, 2, 1])

    predictions = tf.reshape(predictions, [-1, num_anchors * dim, bbox_attrs])

    stride = (img_size[0] // grid_size[0], img_size[1] // grid_size[1])

    anchors = [(a[0] / stride[0], a[1] / stride[1]) for a in anchors]

    box_centers, box_sizes, confidence, classes = tf.split(
        predictions, [2, 2, 1, num_classes], axis=-1)

    box_centers = np.maximum(box_centers, 0)
    box_centers = np.minimum(box_centers, 1)

    grid_x = tf.range(grid_size[0], dtype=tf.float32)
    grid_y = tf.range(grid_size[1], dtype=tf.float32)
    a, b = tf.meshgrid(grid_x, grid_y)

    x_offset = tf.reshape(a, (-1, 1))
    y_offset = tf.reshape(b, (-1, 1))

    x_y_offset = tf.concat([x_offset, y_offset], axis=-1)
    x_y_offset = tf.reshape(tf.tile(x_y_offset, [1, num_anchors]), [1, -1, 2])

    box_centers = box_centers + x_y_offset
    box_centers = box_centers * stride

    anchors = tf.tile(anchors, [dim, 1])
    box_sizes = tf.exp(box_sizes) * anchors
    box_sizes = box_sizes * stride

    detections = tf.concat([box_centers, box_sizes, confidence], axis=-1)

    predictions = tf.concat([detections, classes], axis=-1)
    return predictions


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
