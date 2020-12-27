CC := gcc
CFLAGS := -Wall
LIBS := -lm -lX11 -lXext 

SRC := $(wildcard src/core/*.c)
OBJ := $(SRC:src/%.c=obj/%.o)

SRC_TESTS := $(wildcard src/tests/*.c)
OBJ_TESTS := $(SRC_TESTS:src/%.c=obj/%.o)

.PHONY: compile clean run deploy

obj/%.o: src/%.c
	@echo "Compiling shared library $<..."
	@$(CC) -c -fPIC -o $@ $< $(CFLAGS)

compile: $(OBJ) $(OBJ_TESTS)
	@echo "Linking..."
	@$(CC) -o bin/app $^ $(LIBS)

# deploy the library
deploy: $(OBJ)
	@echo "Linking shared library..."
	@$(CC) -shared -o bin/shared/libx11framework.so $^ $(LIBS)

clean:
	@echo "Cleaning up..."
	@rm -f obj/core/*.o obj/tests/*.o
	@rm -f bin/app bin/shared/libx11framework.so

run:
	@echo "Running newest build..."
	@./bin/app
