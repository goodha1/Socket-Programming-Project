
all : client.c edge.c server_or.c server_and.c
		gcc -o client client.c
		gcc -o edgeserver edge.c
		gcc -o serveror server_or.c
		gcc -o serverand server_and.c

server_or: 
		./serveror

server_and:
		./serverand

edge:
		./edgeserver
clean:
	$(RM) client edgeserver serverand serveror
	
.PHONY: server_or server_and edge client clean

