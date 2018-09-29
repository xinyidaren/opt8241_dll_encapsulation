import ctypes
import cv2
import numpy as np
pyarray = [0.0]*320*240
carray = (ctypes.c_float*len(pyarray))(*pyarray)
so = ctypes.cdll.LoadLibrary
lib = so("./libDepthCapture.so")
print ('init()')
lib.init()
print ('get_img()')
while(1):
    lib.get_img(carray, len(pyarray))
    img = np.array(carray)
    img = np.reshape(img,(240,320))
    cv2.imshow("dd", img)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break