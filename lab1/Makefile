CC=gcc
NAME=test

compile: allocator.c lib.h lib.c
	$(CC) allocator.c lib.c -o $(NAME)

run:
	./$(NAME)

clean:
	rm $(NAME)

all: compile
