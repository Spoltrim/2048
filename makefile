main:
	mkdir bin
	gcc -c src/display.c
	gcc -o bin/main main.c

clean:
	rm -R bin build