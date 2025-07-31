all:
	gcc -o bin/main src/main.c -Iinclude -Llib -lraylib -lm
	./bin/main
