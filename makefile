# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -std=c++11

# SDL2 and SDL_mixer libraries
LIBS = -lSDL2 -lSDL2_mixer -lncurses 

# Source files
SRCS = main.cpp

# Output executable
TARGET = stmp

# Default rule
all: $(TARGET)

# Rule to link the object file and create the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# Clean rule to remove the executable
clean:
	rm -f $(TARGET)

