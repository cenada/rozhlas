all: rozhlas

rozhlas: ../rozhlas.cpp ../modbus.cpp ../modbus.h ../serial.h serial_linux.cpp
	gcc -o rozhlas ../rozhlas.cpp ../modbus.cpp serial_linux.cpp -g

clean:
	rm rozhlas
