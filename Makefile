CC = gcc
CFLAGS = -Wall -Wextra -g
test: test.c
arrange: arrange.c

all: test arrange
	@echo done
clean:
	rm -f test arrange
