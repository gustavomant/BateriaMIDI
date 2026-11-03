CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LDLIBS = -ljack -pthread

TARGET = bateria-midi-bridge

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
