horizonTracker: horizonTracker.cpp i2c.o
	g++ horizonTracker.cpp -o horizonTracker -std=c++11 -lpthread -Ofast `pkg-config --cflags --libs opencv`

i2c.o: i2c.cpp
	g++ i2c.cpp -o i2c.o

clean:
	rm horizonTracker i2c.o
