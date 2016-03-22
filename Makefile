all: clean run

clean:
		rm main

build:
		clang -o main *.c

run: build
		./main
