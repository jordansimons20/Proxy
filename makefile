CFLAGS = -Wall -Wextra -std=c99 -pthread

all: my-proxy

my-proxy: infrastructure.o  proxy.o control.o #next_file.o
	cc -o my-proxy infrastructure.o proxy.o control.o #next_file.o

infrastructure.o: infrastructure.c proxy.h

proxy.o: proxy.c proxy.h

control.o: control.c proxy.h
#next_file.o: next_file.c proxy.h

clean:
	rm -f my-proxy infrastructure.o proxy.o control.o #next_file.o
