MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/scalable

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling scalable example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/scalable.o
	@echo "Building scalable example..."
	@$(CC) -o $(MAKE_DIR)/scalable $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework
