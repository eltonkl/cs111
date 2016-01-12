NAME = EltonLeongAndJamesWang
OPTIMIZE =
DEBUG = -g
CC = gcc
CFLAGS = $(OPTIMIZE) $(DEBUG) -Wall -Wextra -march=native -mtune=native -std=gnu11

default: simpsh

simpsh: main.c options.c handlers.c simpsh.c
	$(CC) $(CFLAGS) $^ -o $@
check: simpsh
	./test.sh
dist: check
	tar -czf lab1-$(NAME).tar.gz --mode=a-w *.c *.h Makefile
clean: 
	rm -f *.o *.tar.gz simpsh
