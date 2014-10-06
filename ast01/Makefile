# Makefile
# Based on the sample file from this website
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

SRC_DIR = src
OBJ_DIR = obj

# output bin and required files
M = matrix
M_TMP_FILES = mPipe
M_OBJ_N = \
	  matrix.o \
	  matrixOps.o
M_OBJS = $(addprefix $(OBJ_DIR)/,$(M_OBJ_N))

CC = gcc
CFLAGS = -g -I$(SRC_DIR)

all : ${M}
	@echo All done

clean :
	@rm -f ${M} ${M_TMP_FILES}
	@rm -rf $(OBJ_DIR)
	@echo Clean done

${M} : $(M_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/matrix.o : $(SRC_DIR)/matrixOps.h

