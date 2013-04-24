#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h> //defined sockaddr_in
#include <strings.h> //bzero
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#define BSIZE 5000
/*Path is hard-coded here*/
#define path /home/elena/school/NetProgramming/httpserver/
char *query(char *host, char *resource){
	//malloc - allocate a block of size specified as argument
	//malloc returns the pointer to the beginning of the block
	//snprintf - safe sprintf, protects from buffer overflows
    //char *query_str ="GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n"; 
	int length = 0;
    char *query_str ="GET /%s HTTP/1.1\r\nHost: %s\r\nIam: oate1\r\n\r\n"; 
	char *q = malloc(snprintf(NULL, 0, query_str, resource, host) + 1);
	sprintf(q, query_str, resource, host);
	return q;	
}


int main(int argc, char *argv[]){
	
	int sockfd, n, nbytes, len, nbytes_read=0, total=0;
	int k=0, count=0, i=0;
	char *request;
    struct addrinfo hints, *res, *r;	
	char buf[80], buf2[BSIZE];
	char *protocol, *host, *port, *file;
	char *ptr, *ptr1;
	int cont_len = 50000;
	char temp[10];	


	if (argc != 2){
		fprintf(stderr, "Enter program name followed by URL.\n");
		return 1;
	}

	
	//checking if the entered URL has a port number 
	//by counting amount of ":" in URL
	while (argv[1][k] != '\0'){
		if (argv[1][k] == ':'){
			count++;
		}
		k++;
	}

	//parsing URL
	//strtok returns pointer to the first char after the pattern
	strtok(argv[1], "/");
	if (count >= 2){
		host = strtok(NULL, ":");
		host++;
		port = strtok(NULL, "/");
	}
	else{
		host = strtok(NULL, "/");
		port = "80";
	}
	file = strtok(NULL, "\0");

	printf("\nConnecting to host: %s, port: %s, file to download: %s\n\n", host, port, file);


	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
		
	n = getaddrinfo(host, port, &hints, &res);
	if (n != 0){
		fprintf(stderr, "Error tcp_connecting for %s, %s\n", host, port);
		return -1;		
	}

	
	//*************************connecting to server
	bzero(buf, sizeof(buf));
	for(r = res; r != NULL; r = r->ai_next){
		if ((sockfd = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) < 0){
			perror("Socket error\n");
			continue;	
		}				

		if ((connect(sockfd, r->ai_addr, r->ai_addrlen)) < 0){		
			perror("Connect error\n");
			close(sockfd);
		}else{
			break;
		}
		
	}
	
	if (r == NULL){
		fprintf(stderr, "Connect error for all available addresses for host: %s, port: %s\n", host, port);
//	fclose(doc);
		return -1;
	}else{
			struct sockaddr_in *sin = (struct sockaddr_in *)r->ai_addr;
			const char *ret = inet_ntop(r->ai_family, &(sin->sin_addr), buf, sizeof(buf));
			if (ret == NULL){
				perror("Failed transforming network to printable address\n");
			} else {
				printf("The address of the server to which we connect: %s\n\n", ret);
			}
		}
	

	
	//***********************Sending request
	request = query(host, file);

	nbytes = send(sockfd, request, strlen(request), 0);	
	if (nbytes < 0 ){
		perror("Send request error\n");
		return -1;
	}
	//**********************Receiving reply
		
	bzero(buf2, sizeof(buf2));

	/*
	//opening a file to read data in
	FILE *doc = fopen(file, "w+");

	while (total < cont_len){
		nbytes_read = read(sockfd, buf2, sizeof(buf2));
		if (nbytes_read == 0){
			fprintf(stderr, "Connection closed\n");
			break;
		}
		if(nbytes_read<0){
			perror("Error reading");
			return -1;
		}
		//looking for first Content-Length pattern in the data returned
		if ((ptr1 = strstr(buf2, "Content-Length: "))!=NULL && total==0){								
			ptr1 = ptr1 + 16;
			//parsing content length from the headers
			while (*ptr1 != '\r'){
			temp[i] = *ptr1;
			i++;
			ptr1++;
			cont_len = strtol(temp, NULL, 10);
			
		}
		}
		if ((ptr=strstr(buf2, "\r\n\r\n"))!=NULL){
			ptr = ptr + 4;

			fprintf(doc, "%s", ptr);
		} else {
			fprintf(doc, "%s", buf2);
		}
		total += nbytes_read; 	
	}
	printf("File downloaded: %s, bytes: %d\n", file, total);
	fclose(doc);*/
	freeaddrinfo(res);
	close(sockfd);

	return 0;
}



