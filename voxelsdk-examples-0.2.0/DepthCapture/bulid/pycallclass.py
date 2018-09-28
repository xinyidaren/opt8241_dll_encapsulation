import ctypes
so = ctypes.cdll.LoadLibrary
lib = so("./libDepthCapture.so")
print ('init()')
lib.init()
print ('get_img')
lib.get_img()

