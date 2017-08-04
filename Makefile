horizonTracker: horizonTracker.cpp
	g++ horizonTracker.cpp -o horizonTracker -std=c++11 -lpthread -O3 `pkg-config --cflags --libs opencv`

clean:
	rm horizonTracker
