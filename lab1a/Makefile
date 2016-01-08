NAME = EltonLeongAndJamesWang
OPTIMIZE =
DEBUG = -g
CC = gcc
CFLAGS = $(OPTIMIZE) $(DEBUG) -Wall -Wextra -march=native -mtune=native -std=c11

default: simpsh

simpsh: main.c options.c functions.c
	$(CC) $(CFLAGS) $^ -o $@
check:

dist: check
	tar -czf lab1-$(NAME).tar.gz --mode=a-w *.c *.h Makefile
clean: 
	rm -f *.o *.tar.gz simpsh
