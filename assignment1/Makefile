.bin/triangles: src/main.cpp src/glad.c
	mkdir -p bin
	g++ -g -std=c++1z -I include/ -o ./bin/assignment1 src/glad.c src/main.cpp -lglfw3 -lX11 -lpthread -lXrandr -lXi -lGL -ldl -L/usr/local/lib -lnanogui

.PHONY: clean run
clean:
	rm -rf bin/

run: ./bin/assignment1
	cd bin/ && ./assignment1
