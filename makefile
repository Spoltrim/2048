main: init build/input_manager build/display
	mkdir bin
	gcc -o bin/main main.c build/input_manager.o build/display.o


init:
	mkdir bin build

build/input_manager:
	gcc -c src/input_manager.c -o build/input_manager.o

build/display:
	gcc -c src/display.c -o build/display.o


clean:
	rm -R bin build