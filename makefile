CFLAGS = -Wall -Wextra -std=c99 -pthread -g

all: my-proxy

my-proxy: infrastructure.o  proxy.o control.o lex.yy.o parse.o #next_file.o
	cc -o my-proxy infrastructure.o proxy.o control.o lex.yy.o -ll parse.o #next_file.o

infrastructure.o: infrastructure.c proxy.h

proxy.o: proxy.c proxy.h

control.o: control.c proxy.h

lex.yy.c: http.l
	flex -i -I http.l

lex.yy.o: lex.yy.c proxy.h

parse.o: parse.c proxy.h
#next_file.o: next_file.c proxy.h

clean:
	rm -f my-proxy infrastructure.o proxy.o control.o lex.yy.o parse.o lex.yy.c #next_file.o
