MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/text

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling text example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/text.o
	@echo "Building text example..."
	@$(CC) -o $(MAKE_DIR)/text $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework
