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


int count1 = 0;
int count2 = 0;
int andflag = 0;
int orflag = 0;

void binary(int n)
{
	if(n>=2)
		binary(n/2);
	printf("%d",n%2);
}
/*code from beej*/
void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET){
		return&(((struct sockaddr_in*)sa)->sin_addr);
	}
	return&(((struct sockaddr_in6*)sa)->sin6_addr);
}
/*end of code from beej*/

int background(int temp[2], int flag){

	int data[2];
	int nowflag;
	data[0] = temp[0];
	data[1] = temp[1];
	/*printf("data %d\n", data[0]);
	printf("data %d\n", data[1]);*/
	char* backserver;
	if(flag == 0){
		backserver = PORT_OR;
		nowflag = orflag;
		orflag++;
	}
	else{
		backserver = PORT_AND;
		nowflag = andflag;
		andflag++;
	}
	/*printf("port number: %s \n", backserver);*/
	int result = 0;
    int socketfd = 0;
    int rv;
    /*code from beej*/
	struct addrinfo  *serverinfo, *p, hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_DGRAM;	
	rv = getaddrinfo(HOST, backserver, &hints, &serverinfo);
	if (rv) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = serverinfo; p != NULL; p = p->ai_next) {
		socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socketfd +1 == 0) {
			perror("server: socket");
			continue;
		}
		break;
	}
	if (!p) {
		fprintf(stderr, "server: failed to bind socket\n");
		exit(1);
	}
	/*end of code from beej*/
	if(nowflag == 0){
		if(flag == 0){
			sendto(socketfd, (char *)& count1, sizeof (count1), 0, p->ai_addr,p->ai_addrlen);
		}
		else{
			sendto(socketfd, (char *)& count2, sizeof (count2), 0, p->ai_addr,p->ai_addrlen);
		}

	}
	sendto(socketfd,(const char *)& data, sizeof (data), 0, p->ai_addr,p->ai_addrlen);
	recvfrom(socketfd, (char *)& result, sizeof (result), 0 , NULL, NULL);
	return result;
}
int main(void){
	/*code from beej*/
	struct addrinfo  *serverinfo, *p, hints; 
    struct sockaddr_storage their_addr; 
    char s[INET6_ADDRSTRLEN];
    socklen_t sin_size;
    int yes=1, rv,socketfd, serverfd2;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE; 
 
    rv = getaddrinfo(HOST, PORT_CLIENT, &hints, &serverinfo);
    if (rv) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
	}

	for (p = serverinfo; p != NULL; p = p->ai_next) {
		socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socketfd +1 == 0) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				+ 1 == 0) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(socketfd, p->ai_addr, p->ai_addrlen) +1 == 0) {
			close(socketfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	if (!p) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	freeaddrinfo(serverinfo); 

	if (listen(socketfd, BACKLOG) + 1 == 0) {
		perror("listen");
		exit(1);
	}
	/*end of code from beej*/
	printf( "The edge server is up and running.\n");
	while(1){
		sin_size = sizeof their_addr;	
		serverfd2 = accept(socketfd, (struct sockaddr *) &their_addr, &sin_size);
		if (serverfd2 +1 == 0) {
			perror("accept");			
			exit(1);
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *) &their_addr), s, sizeof s);
		int total_num;
		int i, j;	
		recv(serverfd2, (char *)&total_num, sizeof (total_num), 0);
		total_num = 3 * total_num;
		j = total_num/3;
		int data[total_num];
		recv(serverfd2, (char *)&data, sizeof (data), 0 );
		printf( "The edge server has received %d lines from the client using TCP over port %s \n", j, PORT_CLIENT);
		int flag, temp[2];

		int result[j], pattern[j];
		for(i = 0; i < j; i++){
			flag = data[3*i];
			if(flag == 0){
				count1++;
			}
			else{
				count2++;
			}
		}
		/*printf("%d\n", count1);
		printf("%d\n", count2);*/

		for(i = 0; i < j; i++){
			temp[0] = data[3*i + 1];
			temp[1] = data[3*i + 2];
			flag = data[3*i];
			pattern[i] = flag;
			result[i] = background(temp,flag);
		}
		printf("The edge server has successfully sent %d lines to Backend-Server OR.\n",count1);
		printf("The edge server has successfully sent %d lines to Backend-Server AND.\n",count2);
		printf("The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP port %s.\n",PORT_EDGE);
		printf("The computation results are:\n");
		for(i = 0; i < j; i++){
			if(pattern[i] == 0){
				binary(data[3*i + 1]);
				printf(" or ");
				binary(data[3*i + 2]);
				printf(" = ");
				binary(result[i]);
				printf("\n");
			}
		}
		for(i = 0; i < j; i++){
			if(pattern[i] == 1){
				binary(data[3*i + 1]);
				printf(" and ");
				binary(data[3*i + 2]);
				printf(" = ");
				binary(result[i]);
				printf("\n");
			}
		}
		printf("The edge server has successfully finished receiving all computation results from the Backend-Server OR and Backend-Server AND.\n");
		send(serverfd2, (const char *)&result, sizeof(result), 0);
		printf("The edge server has successfully finished sending all computation results to the client.\n");
		close(serverfd2); 
	}
}
