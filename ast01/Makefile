# Makefile
# Based on the sample file from this website
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

# Output file
LINK_TARGET = matrix

SRC_DIR = src

# Objects to build
OBJ_DIR = obj
OBJ_NAMES = \
	    matrix.o \
	    matrixOps.o
OBJS = $(addprefix $(OBJ_DIR)/,$(OBJ_NAMES))

REBUILDABLES = $(OBJS) $(LINK_TARGET)

CC = gcc
CFLAGS = -g -I$(SRC_DIR)

all : $(LINK_TARGET)
	@echo All done

clean :
	@rm -f $(REBUILDABLES)
	@rm -rf $(OBJ_DIR)
	@echo Clean done

$(LINK_TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/matrix.o : $(SRC_DIR)/matrixOps.h

