CFLAGS = -g -Wall
ifeq ($(DEBUG), y)
	CFLAGS += -O0
else
	CFLAGS += -O2
endif
CC = gcc
LD = gcc

all: test_speed parse_json

test_speed: json_parser.o rbtree.o test_speed.o
	$(LD) -o test_speed $^

parse_json: json_parser.o rbtree.o test.o
	$(LD) -o parse_json $^

clean:
	rm -f parse_json test_speed *.o

