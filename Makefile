# The compilers to use
CC = g++

# C++ Compiler flags
CXXFLAGS = -c -Wall -g

# Include and Library directories
LIB = -lIrrlicht -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -lopencv_objdetect

# Separate files based on file endings
CC_SRC = $(wildcard src/*.cc)

# Objects for the individual sources
CC_OBJS = $(addprefix obj/, $(notdir $(CC_SRC:%.cc=%.o)))

all: run

debug: CXXFLAGS += -g
debug: run	

run: $(CU_OBJS) $(CC_OBJS)
	$(CC) $(LIB_DIR) $(LIB) $(CU_OBJS) $(CC_OBJS) -o bin/run

obj/%.o : src/%.cc
	$(CC) $(INC_DIR) $(CXXFLAGS) -o $@ $^
obj/%.o : src/%.cu
	$(NC) $(INC_DIR) $(NXXFLAGS) -o $@ $^

clean:
	rm -f obj/* bin/run
