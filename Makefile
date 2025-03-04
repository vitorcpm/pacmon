CC = g++
CFLAGS = -Wall -g
LDFLAGS = -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_acodec
INCLUDE = -I./include
SRC = main.cpp src/game.cpp src/entities.cpp src/resources.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = pacmon

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

main.o: main.cpp include/game.h include/entities.h include/resources.h
	$(CC) $(CFLAGS) $(INCLUDE) -c main.cpp -o main.o

src/game.o: src/game.cpp include/game.h include/entities.h include/resources.h
	$(CC) $(CFLAGS) $(INCLUDE) -c src/game.cpp -o src/game.o

src/entities.o: src/entities.cpp include/entities.h include/game.h
	$(CC) $(CFLAGS) $(INCLUDE) -c src/entities.cpp -o src/entities.o

src/resources.o: src/resources.cpp include/resources.h include/game.h
	$(CC) $(CFLAGS) $(INCLUDE) -c src/resources.cpp -o src/resources.o

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean