MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/snake

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling snake example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/snake.o
	@echo "Building snake example..."
	@$(CC) -o $(MAKE_DIR)/snake $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework
