CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

Fish-Engine: main.cpp
	g++ $(CFLAGS) -o game main.cpp $(LDFLAGS)

.PHONY: test clean

test: game
	./game

clean:
	rm -f game

