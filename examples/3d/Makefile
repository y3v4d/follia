MAKE_DIR := $(PROJECT_DIRECTORY)/build/examples/3d

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/src/%.o: %.c
	@echo "Compiling 3d example..."
	@mkdir -p $(dir $@)
	@$(CC) -c -o $@ $< $(INC) $(CFLAGS)

compile: $(MAKE_DIR)/src/3d.o
	@echo "Building 3d example..."
	@$(CC) -o $(MAKE_DIR)/3d $^ -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework -lm
