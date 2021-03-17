MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/mouse

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling mouse example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/mouse.o
	@echo "Building mouse example..."
	@$(CC) -o $(MAKE_DIR)/mouse $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework
