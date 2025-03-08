CC = gcc
STD_FLAGS = -Wall -Werror -Wextra -std=c11

all: function.o main.o user.o
	$(CC) $(STD_FLAGS) $^ -lm -o task_1_1.out

function.o: function.c
	$(CC) $(STD_FLAGS) -c $<

main.o: main.c
	$(CC) $(STD_FLAGS) -c $<

user.o: user.c
	$(CC) $(STD_FLAGS) -c $<
	
clean:
	rm -f *.out *.o

rebuild: clean all