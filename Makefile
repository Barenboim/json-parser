CFLAGS = -std=c99 -Wall
ifeq ($(DEBUG), y)
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

LD = cc

all: test_speed parse_json

json_parser.o: json_parser.c json_parser.h list.h

test_speed: json_parser.o test_speed.o
	$(LD) -o test_speed $^

parse_json: json_parser.o test.o
	$(LD) -o parse_json $^

clean:
	rm -f parse_json test_speed *.o

