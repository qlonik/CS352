# Makefile
# Based on the sample file from this website
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html

#SRC_DIR = src
#OBJ_DIR = obj

# output bin and required files
SRC_C = sudoku.c
BIN = sudoku                   # mutli threaded sudoku solver
#BIN_OBJ_N = sudoku.o
#BIN_OBJS = $(addprefix $(OBJ_DIR)/,$(BIN_OBJ_N))

BIN_ST = sudoku_st             # single threaded sudoku solver
#BIN_ST_OBJ_N = sudoku_st.o
#BIN_ST_OBJS = $(addprefix $(OBJ_DIR)/,$(BIN_ST_OBJ_N))

BIN_DEBUG = sudoku_debug       # mt sudoku solver with debug msgs
#BIN_DEBUG_OBJ_N = sudoku_debug.o
#BIN_DEBUG_OBJS = $(addprefix $(OBJ_DIR)/,$(BIN_DEBUG_OBJ_N))

BIN_ST_DEBUG = sudoku_st_debug # st sudoku solver with debug msgs
#BIN_ST_DEBUG_OBJ_N = sudoku_st_debug.o
#BIN_ST_DEBUG_OBJS = $(addprefix $(OBJ_DIR)/,$(BIN_ST_DEBUG_OBJ_N))


CC = gcc
CFLAGS = -g -lm -pthread
#CFLAGS = -g -lm -pthread -I$(SRC_DIR)
DST = -DST
DDEBUG = -DPRINT_EACH_STEP

# Create all tasks
all : $(BIN) $(BIN_ST) $(BIN_DEBUG) $(BIN_ST_DEBUG)
	@echo All done

# Clean all
clean :
	@echo "Cleaning $(BIN)"
	@rm -f $(BIN)
	@echo "Cleaning $(BIN_ST)"
	@rm -f $(BIN_ST)
	@echo "Cleaning $(BIN_DEBUG)"
	@rm -f $(BIN_DEBUG)
	@echo "Cleaning $(BIN_ST_DEBUG)"
	@rm -f $(BIN_ST_DEBUG)
	@echo Clean done

$(BIN) : $(SRC_C)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_ST) : $(SRC_C)
	$(CC) $(CFLAGS) $(DST) -o $@ $^

$(BIN_DEBUG) : $(SRC_C)
	$(CC) $(CFLAGS) $(DDEBUG) -o $@ $^

$(BIN_ST_DEBUG) : $(SRC_C)
	$(CC) $(CFLAGS) $(DST) $(DDEBUG) -o $@ $^
