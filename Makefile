	horizonTracker: horizonTracker.cpp i2c.o uart.o
	g++ horizonTracker.cpp i2c.o uart.o -o horizonTracker -std=c++11 -lpthread -Ofast `pkg-config --cflags --libs opencv` $(FLAGS)

serf: FLAGS = -D SERF
serf: horizonTracker


i2c.o: i2c.cpp
	g++ -c i2c.cpp -o i2c.o

uart.o: uart.cpp
	g++ -c uart.cpp -o uart.o

clean:
	rm horizonTracker i2c.o uart.o
