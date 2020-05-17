
BUILD_DIR	?= .
OBJ_DIR		?= ./build/obj
SRC_DIR		?= ./src

OUT			 = executable.out
OUT_PATH	 = $(addprefix $(BUILD_DIR)/, $(OUT))

SRCSPATH	:= $(shell find $(SRC_DIR) -name *.cpp)
SRCDIRS 	:= $(shell find . -name '*.cpp' -exec dirname {} \; | uniq)
OBJDIRS		:= $(patsubst $(SRC_DIR)%, $(OBJ_DIR)%, $(SRCDIRS))
SRCS 		:= $(patsubst $(SRC_DIR)/%, %, $(SRCSPATH))
OBJS 		:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
CC	 		 = g++

CFLAGS 		 = -O3 -Wall -W -pedantic -std=c++17 -I$(SRC_DIR) -I./lib

LFLAGS_MACOS = -framework openGL -lglfw -L./lib -lneural
LFLAGS_LINUX = -lGL -lGLU -lglfw -ldl -L./lib -lneural

OS 			:= $(shell uname)

all: $(OUT)

$(OUT): buildrepo $(OBJS)
ifeq ($(OS), Darwin)
	echo $(SRCSPATH)
	$(CC) $(OBJS) -o $(OUT_PATH) $(LFLAGS_MACOS)
else
	$(CC) $(OBJS) -o $(OUT_PATH) $(LFLAGS_LINUX)
endif

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp $(SRC_DIR)/%.hpp
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp $(SRC_DIR)/%.h
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)


buildrepo:
	$(call make-repo)

clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) $(OUT_PATH)

define make-repo
        for dir in $(OBJDIRS); \
        do \
                mkdir -p $$dir; \
        done
endef
