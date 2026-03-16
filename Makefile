CC = gcc
CFLAGS = -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_net -lGL -lm 

TARGETS = app-ma app-ed app-bricker

all: $(TARGETS)

app-ma: main-ma.c
	@mkdir -p "./build"
	$(CC) $< $(CFLAGS) -o ./build/$@ 

app-ed: main-ed.c
	@mkdir -p "./build"
	$(CC) $< $(CFLAGS) -o ./build/$@ 

app-bricker: main-bricker.c
	@mkdir -p "./build"
	$(CC) $< $(CFLAGS) -o ./build/$@

clean:
	rm -f ./build/$(TARGETS)
