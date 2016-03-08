NAME = EltonLeongAndJamesWang
OPTIMIZE =
DEBUG = -g
CC = gcc
CFLAGS = $(OPTIMIZE) $(DEBUG) -Wall -Wextra -march=native -mtune=native -std=gnu11
LDFLAGS = -lpthread

default: addtest sltest

addtest: part1/driver.c part1/addtest.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

sltest: part2/driver.c part2/SortedList.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
dist: clean
	mkdir .lab4-$(NAME) && cp -r * .lab4-$(NAME)/ && mv .lab4-$(NAME) lab4-$(NAME) && tar -zcvf lab4-$(NAME).tar.gz lab4-$(NAME) && rm -rf lab4-$(NAME)
clean: 
	rm -f lab4-$(NAME).tar.gz
