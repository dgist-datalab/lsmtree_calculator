all: calculator

CC=gcc

SRCS +=\
	   ./lsmtree_calc.c\

TARGETOBJ =\
		  $(patsubst,%.c,%.o,$(SRCS))\


calculator: lsmtree_calc.o main.c 
	$(CC) -g -o $@ $^ -lm


.c.o :
	$(CC) -g -c $< -o $@ -lm

clean:
	@$(RM) calculator
	@$(RM) *.o
