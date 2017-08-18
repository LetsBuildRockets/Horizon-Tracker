ARCH := $(shell uname -m)
ifeq ($(ARCH), armv6l)
	LIBS = -lbcm2835
endif
horizonTracker: horizonTracker.cpp
	g++ horizonTracker.cpp -o horizonTracker -std=c++11 $(LIBS) `pkg-config --cflags --libs opencv`

flags:
	g++ -g -dM -E - < /dev/null

clean:
	rm horizonTracker i2c.o uart.o
