# Makefile
# Based on the sample file from this website
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

SRC_DIR = src
OBJ_DIR = obj

# output bin and required files
M = matrix_pipe
M_TMP_FILES = mPipe
M_OBJ_N = \
	  matrix_pipe.o \
	  matrixOps.o
M_OBJS = $(addprefix $(OBJ_DIR)/,$(M_OBJ_N))

CC = gcc
CFLAGS = -g -I$(SRC_DIR)

# Create all tasks including matrix
all : ${M}
	@echo All done

# Clean all
clean :
	@echo "Cleaning ${M}"
	@rm -f ${M} ${M_TMP_FILES}
	@rm -rf $(OBJ_DIR)
	@echo Clean done

# Create matrix
${M} : $(M_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Create obj_dir folder
$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

# Compile all c code to objects
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

# Set up requirements
$(OBJ_DIR)/matrix_pipe.o : $(SRC_DIR)/matrixOps.h

