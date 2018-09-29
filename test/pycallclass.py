import ctypes
import numpy as np
so = ctypes.cdll.LoadLibrary
lib = so("./libpycallclass.so")
print ('------------------display()---------------------')
lib.display()
print ('------------------display(100)------------------')
lib.display_int(100)
print ('------------------display_char()------------------')
lib.display_char()
print ('------------------sum()------------------')
print (lib.sum())
print ('------------------add(3,4)------------------')
print (lib.add(3,4))
print ('------------------sum_array(carray, len(pyarray))------------------')

pyarray = [1,2,3,4,5,6,7,8]
carray = (ctypes.c_int*len(pyarray))(*pyarray)
print(lib.sum_array(carray, len(pyarray)))
print ('------------------modify_array(carray, len(pyarray))------------------')
lib.modify_array(carray, len(pyarray))
print (np.array(carray))