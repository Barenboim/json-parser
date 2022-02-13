CFLAGS = -g
CC = gcc
LD = gcc

parse_json : json_parser.o rbtree.o test.o
	$(LD) -o parse_json $^

clean:
	rm -f parse_json *.o

