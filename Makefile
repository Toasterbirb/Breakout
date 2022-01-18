CC=g++
outputDir=./build
WarningFlags=-Wpedantic -pedantic -Wall -Wextra
SDL_FLAGS=-lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx

all: breakout

breakout: breakout.o
	mkdir -p build
	rsync -av ./res ./build/
	$(CC) $^ $(SDL_FLAGS) -lbirb2d $(WarningFlags) -o $(outputDir)/breakout

breakout.o: ./src/breakout.cpp
	$(CC) -O2 -c $^ -o breakout.o

clean:
	rm -rf build
	rm -f *.o
