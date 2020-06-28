
compile: server.c client.c
	gcc server.c -o server -Wall -pthread
	gcc client.c -o client -Wall
