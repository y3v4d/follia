PROJECT_DIRECTORY := $(PWD)

CC := gcc
CFLAGS := -Wall
LIBS := -lm -lX11 -lXext

INC := -I include
SRC := $(shell find src -type f -name *.c)
DEPS := $(shell find include -type f -name *.h)
OBJ := $(SRC:src/%.c=build/src/%.o)

export PROJECT_DIRECTORY

.PHONY: compile clean examples/snake examples/scalable examples/text examples/space-invaders examples/primitives tests/timer tests/mouse

build/src/%.o: src/%.c $(DEPS)
	@echo "Compiling shared library $<..."
	@mkdir -p $(dir $@)
	@$(CC) -c -fPIC -o $@ $< $(INC) $(CFLAGS)

compile: $(OBJ)
	@echo "Building shared library..."
	@mkdir -p build/lib
	@$(CC) -shared -o build/lib/libx11framework.so $^ $(LIBS)
	@echo "Start making examples..."
	@mkdir -p build/logs
	@make -C examples/snake -f Makefile compile
	@make -C examples/scalable -f Makefile compile
	@make -C examples/text -f Makefile compile
	@make -C examples/space-invaders -f Makefile compile
	@make -C examples/primitives -f Makefile compile
	@make -C tests -f Makefile compile

clean:
	@echo "Cleaning up..."
	@rm -r build/*

examples/snake:
	@echo "Running snake example..."
	@./build/examples/snake/snake

examples/scalable:
	@echo "Running scalable example..."
	@./build/examples/scalable/scalable

examples/text:
	@echo "Running text example..."
	@./build/examples/text/text

examples/space-invaders:
	@echo "Running space-invaders example..."
	@./build/examples/space-invaders/space-invaders

examples/primitives:
	@echo "Running primitves example..."
	@./build/examples/primitives/primitives

tests/timer:
	@echo "Running timer test..."
	@./build/tests/timer

tests/mouse:
	@echo "Running mouse test..."
	@./build/tests/mouse
