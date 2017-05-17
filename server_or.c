  #include <stdio.h> 
 #include <stdlib.h> 
 #include <unistd.h> 
 #include <errno.h> 
 #include <string.h> 
 #include <netdb.h> 
 #include <sys/types.h> 
 #include <netinet/in.h> 
 #include <sys/socket.h> 
 #include <arpa/inet.h> 
 #include <sys/wait.h>

#define PORT_OR "21262"   
#define PORT_AND "22262"   
#define PORT_EDGE "24262"
#define BACKLOG 10
#define PORT_CLIENT "23262"   
#define HOST "localhost"


void binary(int n)
{
	if(n>=2)
		binary(n/2);
	printf("%d",n%2);
}

int main(){
	/*code from beej*/
	int socketfd,rv;
	struct addrinfo  *serverinfo, *p, hints;
	socklen_t addr_len;
	struct sockaddr_storage their_addr;


	memset(&hints, 0, sizeof hints);  
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;  


	rv = getaddrinfo(HOST, PORT_OR, &hints, &serverinfo);
	if (rv) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	for (p = serverinfo; p != NULL; p = p->ai_next) {
		socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socketfd +1 == 0) {
			perror("server_and: socket");
			continue;
		}
		if (bind(socketfd, p->ai_addr, p->ai_addrlen) + 1 == 0) {
			close(socketfd);
			perror("server_and: bind");
			continue;
		}
		break;
	}
	if (!p) {
		fprintf(stderr, "serverA: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(serverinfo);

	/*end of code from beej */
	printf( "The Server OR is up and running using UDP on port 21262.\n");
	while(1){
		addr_len = sizeof(their_addr);
		int data[2], count1, i;
		recvfrom(socketfd, (char *)& count1, sizeof count1 , 0,(struct sockaddr *)&their_addr, &addr_len);
		printf( "The Server OR start receiving lines from the edge server for OR computation.\n");
		printf( "The computation results are:\n");
		for(i = 0; i < count1; i++){
			recvfrom(socketfd, (char *)&data, sizeof(data), 0,(struct sockaddr *)&their_addr, &addr_len);
			int result = data[0] | data[1];
			binary(data[0]);
			printf(" or ");
			binary(data[1]);
			printf(" = ");
			binary(result);
			printf("\n");
			sendto(socketfd, (char *)& result, sizeof result , 0,(struct sockaddr *) &their_addr, addr_len);
		}
		printf("The Server OR has successfully received %d lines from the edge server and finished all OR computations.\n", count1);
		printf( "The Server OR has successfully finished sending all computation results to the edge server\n");
	}
}