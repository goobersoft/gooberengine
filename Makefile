
FLAGS = -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_net -lGL

bricker:
	gcc ./main-bricker.c ${FLAGS} -o ./app-bricker

ma:
	gcc ./main-ma.c ${FLAGS} -o ./app-ma