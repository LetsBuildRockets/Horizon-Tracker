horizonTracker: horizonTracker.cpp
	g++ horizonTracker.cpp -o horizonTracker -std=c++11 -lbcm2835 `pkg-config --cflags --libs opencv` -g

clean:
	rm horizonTracker
