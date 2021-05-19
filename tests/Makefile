MAKE_DIR := $(PROJECT_DIRECTORY)/build/tests

CC := gcc
CFLAGS := -Wall

INC := -I $(PROJECT_DIRECTORY)/include
LIB_DIR := $(PROJECT_DIRECTORY)/build/lib

$(MAKE_DIR)/%: %.c
	@mkdir -p $(dir $@)
	@$(CC) -o $@ $< $(INC) $(CFLAGS) -Wl,-rpath=$(LIB_DIR) -L $(LIB_DIR) -lx11framework

compile: $(MAKE_DIR)/timer $(MAKE_DIR)/mouse $(MAKE_DIR)/text
	@echo "Start to compile and build tests..."
