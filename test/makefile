
libpycallclass: pycallclass.o
	g++ -shared -fPIC -o libpycallclass.so pycallclass.o


pycallclass.o: pycallclass.cpp
	g++ -c -fPIC pycallclass.cpp

clean:
	rm -rf *.o
