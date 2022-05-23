.PHONY:all
all:server shclient

shclient:shclient.c comm.c
	gcc -o $@ $^
server:server.c comm.c
	gcc -o $@ $^
.PHONY:clean
clean:
	rm -f chclient server