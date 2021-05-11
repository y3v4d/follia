MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/primitives

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling primitives example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/primitives.o $(MAKE_DIR)/src/slider.o $(MAKE_DIR)/src/checkbox.o $(MAKE_DIR)/src/selector.o
	@echo "Building primitives example..."
	@$(CC) -o $(MAKE_DIR)/primitives $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework
