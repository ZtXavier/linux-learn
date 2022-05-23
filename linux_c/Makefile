.PHONY:all
all:server shclient

shclient:shclient.c comm.c
	gcc -o $@ $^ -g
server:server.c comm.c
	gcc -o $@ $^ -g

.PHONY:clean
clean:
	rm -f chclient server