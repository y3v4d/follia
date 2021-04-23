MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/space-invaders

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling space-invaders example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/space-invaders.o
	@echo "Building space-invaders example..."
	@$(CC) -o $(MAKE_DIR)/space-invaders $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework
