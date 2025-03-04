ifeq ($(OS),Windows_NT)
    # Windows settings (MinGW)
    CC = g++
    CFLAGS = -Wall -g
    LDFLAGS =  -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_acodec
    TARGET = pacmon.exe
    RM = del
else
    # Linux settings
    CC = g++
    CFLAGS = -Wall -g
    LDFLAGS = -lallegro -lallegro_image -lallegro_font -lallegro_ttf -lallegro_audio -lallegro_acodec
    TARGET = pacmon
    RM = rm -f
endif

SOURCES = main.cpp src/game.cpp src/entities.cpp src/resources.cpp
OBJECTS = $(SOURCES:.cpp=.o)
INCLUDE = -Iinclude

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	$(RM) $(OBJECTS) $(TARGET)
else
	$(RM) $(OBJECTS) $(TARGET)
endif

.PHONY: all clean