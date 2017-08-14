ARCH := $(shell uname -m)
ifeq ($(ARCH), armv61)
	LIBS = -lbcm2835
endif
horizonTracker: horizonTracker.cpp
	g++ horizonTracker.cpp -o horizonTracker -std=c++11 $(LIBS) `pkg-config --cflags --libs opencv` -g

clean:
	rm horizonTracker
