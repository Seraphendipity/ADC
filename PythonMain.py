# INITIALIZATION
import cv2
import serial
import PySimpleGUI as sg
import csv
import os
# import matplotlib.pyplot as plt
# import numpy as np
# dir = '../../Programs/'
dir = '' 
camera_id = 0
delay = 1
layout_column_1 = [
    [sg.Text('Speed: '), sg.Text('0', key='motor_speed'), sg.Text('')]
] 
layout_column_2 = [
    [sg.Text('Distance: '), sg.Text('0',key='forward_distance'), sg.Text('cm')]
]
layout_column_3 = [
    [sg.Text('Latch State: '), sg.Text('CLOSED',key='latch_state'), sg.Text('')]
]
layout = [  [sg.Column(layout_column_1), sg.Column(layout_column_2),sg.Push(),sg.Column(layout_column_3)],
            [sg.HorizontalSeparator(color='red')],
            [sg.Text('QR Code Value: '),sg.Text(key='qr_value')],
            [sg.Image(filename='', key='image')],
            [sg.HorizontalSeparator(color='red')],
            [sg.Multiline(key='ML',default_text='Serial Connection Window', size=(None, 5))],
            [sg.Save(), sg.Exit()] ]
            # [sg.Canvas()]
            # [sg.ProgressBar(10, key='-PROGRESS_BAR-')

window_name = 'T4-ADC-Rosebotics GUI'
window = sg.Window(window_name, layout)

qr_detector = cv2.QRCodeDetector()
vid_cap = cv2.VideoCapture(camera_id)

open_serial = True
bmsg = True

## Initializes serial; I do not quite understand syntax but functional it seems.
## 9600 Refers to Baud Rate -- Match on Arduino
## Arduinos are typically '/dev/ttyACM0' or '/dev/ttyUSB0'.
if(open_serial == True):
    if __name__ == '__main__':
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
        ser.reset_input_buffer()

## Import patient data, convert to List of Strings
with open(dir+'patient_names.csv') as csvfile:
    patients_list_table = list(csv.reader(csvfile))
    patients_list = []
    for l in patients_list_table:
        patients_list.append(l[0])
    print(patients_list)

def SaveData(data):
    i = 0
    while os.path.exists(dir+"Data/data-output-%s.txt" % i):
        i += 1
    with open(dir+"Data/data-output-%s.txt" % i, "w") as file:
        file.write(data)

# MAIN
while True:
    # GUI WINDOW
    event, values = window.read(timeout=20)
    ## Q to Quit
    if (cv2.waitKey(delay) & 0xFF == ord('q')) or (event == 'Exit') or (event == sg.WIN_CLOSED):
        break
    ## Save
    if event == 'Save':
        SaveData(values['ML'])

    # SERIAL COM
    if(open_serial == True):
        # Serial
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
        window['ML'].print(line)
        if ( len(line.split()) > 1):
            (firstWord, rest) = line.split(maxsplit=1)
            if(firstWord == 'Distance:'):
                window['forward_distance'].update(rest)
            if(firstWord == 'Motor:'):
                window['motor_speed'].update(rest.split(",")[2])
            if(firstWord == 'SERVO:'):
                window['latch_state'].update(rest)
    


    elif(bmsg != ''):
        window['ML'].print('No Serial Connection Established (open_serial = false).')
        bmsg = ''
    
    # QR Code
    ## Retval determines if it is detected
    ret, frame = vid_cap.read()
    if ret:
        ret_qr, decoded_info, points, _ = qr_detector.detectAndDecodeMulti(frame)
        if ret_qr:
            for qr_value_found, p in zip(decoded_info, points):
                if qr_value_found:
                    window['ML'].print('QR Code Detected: '+qr_value_found)
                    color = (0, 255, 0)
                    print(qr_value_found)
                    if (qr_value_found in patients_list):
                        window['ML'].print('MATCH!')
                        window['qr_value'].update(qr_value_found)
                else:
                    color = (0, 0, 255)
                frame = cv2.polylines(frame, [p.astype(int)], True, color, 8)
        # cv2.imshow(window_name, frame)
        imgbytes = cv2.imencode('.png',frame)[1].tobytes()
        window['image'].update(data=imgbytes)

cv2.destroyWindow(window_name)

