.bin/triangles: src/triangles.cpp src/glad.c
	mkdir -p bin
	g++ -I ./include/ -o ./bin/triangles src/glad.c src/triangles.cpp -lglfw3 -lX11 -lpthread -lXrandr -lXi -ldl

.PHONY: clean run
clean:
	rm -rf bin/

run: ./bin/triangles
	cd bin/ && ./triangles
