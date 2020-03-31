CC=gcc
NAME=test
DEPS=main.c allocator.c struct.c
LIBS=allocator.h struct.h

compile: $(DEPS) $(LIBS)
	$(CC) $(DEPS) -o $(NAME)

run:
	./$(NAME)

clean:
	rm $(NAME)

all: compile
