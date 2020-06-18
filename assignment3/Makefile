.bin/triangles: src/main.cpp src/glad.c
	mkdir -p bin
	g++ -g -std=c++1z -I include/ -o ./bin/assignment3 src/glad.c src/main.cpp -lglfw3 -lX11 -lpthread -lXrandr -lXi -lGL -ldl -L/usr/local/lib -lnanogui

.PHONY: clean run
clean:
	rm -rf bin/

run: ./bin/assignment3
	cd bin/ && ./assignment3
