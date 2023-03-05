
BUILD_DIR	?= .
OBJ_DIR		?= ./build/obj
SRC_DIR		?= ./src

OS 		:= $(shell uname)

SOURCES := $(wildcard **/*.cpp)
SOURCES := $(filter-out main%, $(SOURCES))


#SRCSPATH	:= $(shell find $(SRC_DIR) -name "*.cpp")
#SRCSPATH := $(shell find $(SRC_DIR) -name "*.cpp" | grep -v '^$(SRC_DIR)/main')
SRCSPATH := $(shell find $(SRC_DIR) -name "*.cpp" -not -path "$(SRC_DIR)/main*")


SRCDIRS 	:= $(shell find . -name '*.cpp' -exec dirname {} \; | uniq)
OBJDIRS		:= $(patsubst $(SRC_DIR)%, $(OBJ_DIR)%, $(SRCDIRS))
SRCS 		:= $(patsubst $(SRC_DIR)/%, %, $(SRCSPATH))
OBJS 		:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
CC	 	 	 = g++

MAIN_OBJ = $(OBJ_DIR)/main.o

ifeq ($(OS), Darwin)
CFLAGS = -O3 -Wall -W -std=c++17 -I$(SRC_DIR) -g
LFLAGS = -framework openGL -lglfw -g
EXT    = .out
else ifeq ($(OS), Linux)
CFLAGS = -O3 -Wall -W -std=c++17 -I$(SRC_DIR)
LFLAGS = -lGL -lGLU -lglfw -ldl
EXT    = .out
else
CFLAGS = -O0 -Wall -W -std=c++17 -I$(SRC_DIR) -IC:\\libs\\headers -g
LFLAGS = -lopengl32 -lglfw3 -lgdi32 -LC:\\libs\\dlls -g
EXT    = .exe
endif


all: buildrepo app
noise: buildrepo app_noise

app: $(OBJS) $(OBJ_DIR)/main.o
	$(CC) $(OBJS) $(OBJ_DIR)/main.o -o $(BUILD_DIR)/$@$(EXT) $(LFLAGS)

app_noise: $(OBJS) $(OBJ_DIR)/main_noise.o
	$(CC) $(OBJS) $(OBJ_DIR)/main_noise.o -o $(BUILD_DIR)/$@$(EXT) $(LFLAGS)
	

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp $(SRC_DIR)/%.hpp
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp $(SRC_DIR)/%.h
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

buildrepo:
	@$(call make-repo)

clean:
	rm -f $(OBJS) $(OBJ_DIR)/*.o

cleanall:
	rm -f $(OBJS) $(OBJ_DIR)/*.o $(BUILD_DIR)/*$(EXT)

define make-repo
        for dir in $(OBJDIRS); \
        do \
                mkdir -p $$dir; \
        done
endef
