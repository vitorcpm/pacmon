CC = g++
CFLAGS = -Wall -std=c++11
LDFLAGS = -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_acodec

SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = obj

SOURCES = main.cpp $(SRC_DIR)/game.cpp $(SRC_DIR)/map.cpp $(SRC_DIR)/entity.cpp $(SRC_DIR)/hunter.cpp
OBJECTS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SOURCES)))
EXECUTABLE = pacmon

$(shell mkdir -p $(OBJ_DIR))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/main.o: main.cpp
	$(CC) -c $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)

.PHONY: clean