CC=gcc
NAME=test
DEPS=main.c allocator.c slab.c

compile: $(DEPS)
	$(CC) $(DEPS) -o $(NAME)

debug: $(DEPS)
	$(CC) -D DEBUG=1 $(DEPS) -o $(NAME)

run:
	./$(NAME)

clean:
	rm $(NAME)

all: compile
