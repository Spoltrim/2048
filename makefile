main: build/input_manager
	mkdir bin
	gcc -o bin/main main.c build/input_manager.o


build/input_manager:
	mkdir build
	gcc -c src/input_manager.c -o build/input_manager.o


clean:
	rm -R bin build