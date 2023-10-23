CFLAGS = -g -Wall -std=c99
LDFLAGS = -lm
ifeq ($(DEBUG), y)
	CFLAGS += -O0
else
	CFLAGS += -O2
endif
CC = gcc
LD = gcc

all: test_speed parse_json

json_parser.o: json_parser.c json_parser.h list.h rbtree.h

test_speed: json_parser.o rbtree.o test_speed.o
	$(LD) -o test_speed $^ $(LDFLAGS)

parse_json: json_parser.o rbtree.o test.o
	$(LD) -o parse_json $^ $(LDFLAGS)

clean:
	rm -f parse_json test_speed *.o

