CFLAGS = -Wall -Wextra -std=c99

all: my-proxy

my-proxy: proxy.o #next_file.o
	cc -o my-proxy proxy.o #next_file.o

proxy.o: proxy.c proxy.h

#next_file.o: next_file.c proxy.h

clean:
	rm -f proxy proxy.o #next_file.o
