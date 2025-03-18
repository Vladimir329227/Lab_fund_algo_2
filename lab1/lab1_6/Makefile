CC = gcc
STD_FLAGS = -Wall -Werror -Wextra -std=c11

all: main client
	
main: main.o
	$(CC) $(STD_FLAGS) $^ -lm -o task_1_6.out

client: function.o
	$(CC) $(STD_FLAGS) $^ -lm -o client.out

function.o: function.c
	$(CC) $(STD_FLAGS) -c $<

main.o: main.c
	$(CC) $(STD_FLAGS) -c $<
	
clean:
	rm -f *.out *.o

rebuild: clean all