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
dist: check clean
	cd .. && tar -zcvf lab1-$(NAME)/lab1-$(NAME).tar.gz lab1-$(NAME)/*.c lab1-$(NAME)/*.h lab1-$(NAME)/Makefile lab1-$(NAME)/README lab1-$(NAME)/*.sh lab1-$(NAME)/*/*
clean: 
	rm -f *.o *.tar.gz simpsh
