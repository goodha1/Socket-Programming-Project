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

#define PORT "23262"  
#define HOST "localhost"


void binary(int n)
{
	if(n>=2)
		binary(n/2);
	printf("%d",n%2);
}


int main(int argc, char* argv[]){
	int total_num = 0;
	FILE *fp = NULL;
	fp = fopen(argv[1],"r");
	if(!fp){
		fprintf(stderr, "Error: Can not open input file\n");
		exit(1);
	}
	char temp[100];
	temp[99] = '\0';
	temp[0] = '\0';
	char *start,*stop;
	char *and = "and";
	char *or = "or";
	int flag;
	int data[300];
  	while((fgets(temp, sizeof(temp), fp)) != NULL)
    {  	
    	/*printf("%d\n",total_num);
    	printf("%s",temp);*/
    	if(temp[0] == '\n'){
    		break;
    	}
    	start = temp;
    	stop = strchr(start, ',');
    	*stop++ = '\0';
    	if(strcmp(start, and) == 0){
 
    		flag = 1;
    	}
    	else if(strcmp(start, or) == 0){
    		
    		flag = 0;
    	}
    	else{
    		fprintf(stderr, "Error: Invalid input\n");
    		exit(1);
    	}
    	data[3*total_num] = flag;
    	start = stop;
    	stop = strchr(start, ',');
    	*stop++ = '\0';
    	data[3*total_num +1] = strtol( start, NULL, 2 );
    	start = stop;
    	stop = strchr(start, '\n');
    	*stop++ = '\0';
    	data[3*total_num +2] = strtol( start, NULL, 2 );
    	/*printf("%d\n", data[3*total_num +2]);    	
    	printf("%s",temp);*/

		total_num++;
    }
    int realnum = 3*total_num;
    int realdata[realnum];
    int i;
    for(i = 0; i < realnum; i++){
    	//printf("test\n");
    	realdata[i] = data[i];
    	/*printf("%d ", realdata[i]);*/
    }
    /*printf("%d\n",total_num);*/
    fclose(fp);
    /*code from beej*/
    int socketfd = 0;
    int rv;
	struct addrinfo  *serverinfo, *p, hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	rv = getaddrinfo(HOST, PORT, &hints, &serverinfo);
	if (rv) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	for (p = serverinfo; p != NULL; p = p->ai_next) {
		socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socketfd +1 == 0) {
			perror("client: socket");
			continue;
		}

		if (connect(socketfd, p->ai_addr, p->ai_addrlen) + 1 == 0) {
			close(socketfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (!p) {
		fprintf(stderr, "client: failed to connect. \n");
		exit(1);
	
	}
	/*end of code from beej*/
	freeaddrinfo(serverinfo); 
	printf("The client is up and running. \n");

	send(socketfd, (char *)& total_num, sizeof (total_num), 0);
	send(socketfd, (char *)& realdata, sizeof (realdata), 0);

	printf("The client has successfully finished sending %d lines to the edge server.\n", total_num);
	int result[total_num];
	recv(socketfd, (char *)&result, sizeof (result), 0);
	printf("The client has successfully finished receiving all computation results from the edge server.\n");
	printf("The final computation results are:\n");
	for(i = 0; i < total_num; i++){
		binary(result[i]);
		printf("\n");
	}
	close(socketfd);
	return 0;
}