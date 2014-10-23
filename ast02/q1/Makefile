# Makefile
# Based on the sample file from this website
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

SRC_DIR = src
OBJ_DIR = obj

# output bin and required files
BIN = matrix_shm
# M_TMP_FILES = 
BIN_OBJ_N = \
	  matrix_shm.o \
	  matrixOps.o
BIN_OBJS = $(addprefix $(OBJ_DIR)/,$(BIN_OBJ_N))

CC = gcc
CFLAGS = -g -lrt -pthread -I$(SRC_DIR)

# Create all tasks including matrix
all : $(BIN)
	@echo All done

# Clean all
clean :
	@echo "Cleaning $(BIN)"
	@rm -f $(BIN)
	@rm -rf $(OBJ_DIR)
	@echo Clean done

# Create matrix
$(BIN) : $(BIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Create obj_dir folder
$(OBJ_DIR) :
	@mkdir $(OBJ_DIR)

# Compile all c code to objects
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

# Set up requirements
$(OBJ_DIR)/matrix_pipe.o : $(SRC_DIR)/matrixOps.h

