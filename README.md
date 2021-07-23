# PCB Solder Joint Detection System

## Introduction

We are group from NCKU CASLab undergraduate students. We are dedicate to solve the problem of solder joint detecting problem. Since inspecting solder joint defect by human eye is time consuming and low efficiency, we proposed a detection system combine with edge AI and computer. Our device could also work only with power supply and need no computer or laptop connect to it.  Hope this AIoT device could solve the problem people long encountered.

<p align="center">
<img src="./pictures/board.jpg" alt="board" width="400" align="center"/>
</p> 
<p align='center'>Device Image</p>



## User Manual

### Installation

1. Compile and generate image file for uploading to Himax WE-I Plus EVB

   ```bash
   $ cd himax_tflm && make downloads # download himax sdk
   $ cd ARC_SDK
   $ make clean
   $ make && make flash
   ```

   You should see `output_gnu.img` in this folder after this step.

2. Deploy image to Himax WE-I Plus EVB, we use `minicom` and `xmodem` here, connect Himax WE-I Plus EVB to your device first

   ```bash
   $ sudo minicom -s # configure minicom baud rate to 115200
   ```

   ```
   (minicom) press 1 on keyboard and reset on WE-I Plus
   (minicom) press 1 to upload image
   (minicom) select xmodem and choose the image file
   (minicom) after uploading image file, press reset on WE-I Plus
   ```

3. Deploy Arduino pro mini

   - open `tft_display/tft_display.ino`
   - plug in Arduino pro mini and upload the program

4. Computer library settings

   - requirement: Python 3.9.6, pip 20.3.4

   - install pip libraries with

     ````bash
     $ sudo pip install -r requirements.txt
     ````

     

### Usage

- With computer / laptop

  1. Make sure Himax WE-I Plus EVB has been connected to computer through micro USB cable (should be able to transfer data), Arduino is optionally powered

  2. Use `pyserial` to receive messages from Himax

     ```bash
     $ sudo python SerialOut.py
     
     usage: SerialOut.py [-h] [-p PORT] [-b BAUD] [-t THRESH] [-i IOU_THRESH]
     
     optional arguments:
       -h, --help            show this help message and exit
       -p PORT, --port PORT  specify the port of WE-1 Plus
       -b BAUD, --baud BAUD  specify the baud rate of transferring image and predictions
       -t THRESH, --thresh THRESH
                             specify the thresh of detecting
       -i IOU_THRESH, --iou_thresh IOU_THRESH
                             specify the iou thresh of detecting
     
     ```

  3. After start signals were sent from WE-I Plus, one can see the detection result from the screen

  4. To save current inference picture, press `SPACE` to capture; to exit from the program, press `ESC`

- Without computer / laptop

  1. Power up WE-I Plus and Arduino, one should see the QC test result from the TFT display, there were two results:
     1. QC PASSED! - indicates that there were no defected joint detected on the current frame
     2. Defect detected! - indicates that there might have some specification problems on the current frame, the amount of defected joints of each classes would also been shownk 
