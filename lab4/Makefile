NAME = EltonLeongAndJamesWang
OPTIMIZE =
DEBUG = -g
CC = gcc
CFLAGS = $(OPTIMIZE) $(DEBUG) -Wall -Wextra -march=native -mtune=native -std=gnu11

default: 

part1: driver.c

part2: part2/driver.c part2/SortedList.c
	$(CC) $(CFLAGS) $^ -o $@
dist: clean
	mkdir lab4-$(NAME) && cp -r * lab4-$(NAME)/ && tar -zcvf lab4-$(NAME).tar.gz lab4-$(NAME)
clean: 
	rm -f lab4-$(NAME).tar.gz
