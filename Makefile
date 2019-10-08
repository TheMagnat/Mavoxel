
BUILD_DIR	?= ./build
OBJ_DIR		?= ./build/obj
SRC_DIRS	?= ./src

OUT			 = executable.out
OUT_PATH	 = $(addprefix $(BUILD_DIR)/, $(OUT))

SRCSPATH	:= $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
SRCS 		:= $(notdir $(SRCSPATH))
OBJS 		:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
CC	 		 = g++

CFLAGS 		 = -O3 -Wall -W -pedantic -std=c++17

LFLAGS_MACOS = -framework openGL -lglfw
LFLAGS_LINUX = -lGL -lGLU -lglfw -ldl

OS 			:= $(shell uname)

init: createFolder all

createFolder:
	@- mkdir -p $(BUILD_DIR)
	@- mkdir -p $(OBJ_DIR)

all: $(OBJS)
ifeq ($(OS), Darwin)
	$(CC) $(OBJS) -o $(OUT_PATH) $(macLFLAGS)
else
	$(CC) $(OBJS) -o $(OUT_PATH) $(linuxLFLAGS)
endif

$(OBJ_DIR)/%.o:	$(SRC_DIRS)/%.cpp $(SRC_DIRS)/%.hpp
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIRS)/%.cpp $(SRC_DIRS)/%.h
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIRS)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/glad.o: $(SRC_DIRS)/glad.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $(OUT)
