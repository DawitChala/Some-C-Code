CFLAGS =-g -std=gnu99 -Wall -Wextra
all: klient.c server.c
	gcc -g -o klient klient.c 
	gcc -g -o server server.c
klient: klient.c
	gcc -g -o klient klient.c
server: server.c
	gcc -g -o server server.c
clean:
	-rm server
	-rm klient
