PROJECT_DIRECTORY := $(PWD)

CC := gcc
CFLAGS := -Wall
LIBS := -lm -lX11 -lXext

INC := -I include
SRC := $(shell find src -type f -name *.c)
OBJ := $(SRC:src/%.c=build/src/%.o)

export PROJECT_DIRECTORY

.PHONY: compile clean examples/snake

build/src/%.o: src/%.c
	@echo "Compiling shared library $<..."
	@mkdir -p $(dir $@)
	@$(CC) -c -fPIC -o $@ $< $(INC) $(CFLAGS)

compile: $(OBJ)
	@echo "Building shared library..."
	@mkdir -p build/lib
	@$(CC) -shared -o build/lib/libx11framework.so $^ $(LIBS)
	@echo "Start making examples..."
	@make -C examples/snake -f Makefile compile

clean:
	@echo "Cleaning up..."
	@rm -r build/*

examples/snake:
	@echo "Running snake example..."
	@./build/examples/snake/snake
