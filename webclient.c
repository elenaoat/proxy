#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#define BUFSIZE 20000
#define MAXPATH 1024
#include <stddef.h>


/* Download HTTP client */



char *query(char *host, char *resource){

	char *query_str ="GET /%s HTTP/1.1\r\nHost: %s\r\nIam: oate1\r\n\r\n";
	char *q = malloc(snprintf(NULL, 0, query_str, resource, host) + 1);
	sprintf(q, query_str, resource, host);
	return q;
}

int connecting(char *host, char *service){
	int sockfd;
	struct addrinfo hints, *res, *ressave;	
	char address[46];

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype =	SOCK_STREAM;
	
	if (getaddrinfo(host, service, &hints, &res) != 0){
		perror("Error obtaining address\n");
		exit(1);
	}
	ressave = res;
	do {
			if ((sockfd=(socket(res->ai_family, res->ai_socktype, res->ai_protocol))) < 0){
				perror("Socket error");
				continue;
			}

			if ((connect(sockfd, res->ai_addr, res->ai_addrlen)) == 0){	
				struct sockaddr_in *s = (struct sockaddr_in *) res->ai_addr;
				inet_ntop(res->ai_family, &(s->sin_addr), address, sizeof(address));

			/*	printf("The address used for connection: %s\n", address);*/
				break;
			}
			close(sockfd);
		
	} while ((res=res->ai_next) != NULL);
	if (res == NULL){
		fprintf(stderr, "error to create any socket\n");
	} 
	freeaddrinfo(ressave);		
	return(sockfd);
	
}


int main(int argc, char **argv){
	
	int sockfd, nbytes, total=0, headers_length=0, content_length=0, bytes;
	int i, written_content_length;
	char buff[BUFSIZE], buff_copy[BUFSIZE], buff2_copy[BUFSIZE];
	FILE *file;
	char *q;	
	char home[MAXPATH];
	/*char *home = getenv("HOME");*/
	char *double_newline=0;
	int partial_content_size;
	
	getcwd(home, MAXPATH);

	if (argc != 4){
		fprintf(stderr, "usage: a.out IP_address/hostname port/service filename\n");
		return 1;
	}
	
	sockfd = connecting (argv[1], argv[2]);	
	
	/*forming query to send to HTTP server*/
	q = query(argv[1], argv[3]);
	
	if (sockfd < 0){
		perror("Error creating socket\n");
		return 1;
	}	

	/*send request to server*/
   	while ((nbytes=write(sockfd, q, strlen(q))) > 0){
		total += nbytes;

		if (total == strlen(q)){
			break;
		}	
	}
	if (nbytes < 0){
		perror("Send error. Possibly the port or hostname are wrong.\n");
		return 1;
	}
	
	bzero(buff, BUFSIZE);
	bzero(buff_copy, BUFSIZE);
	bzero(buff2_copy, BUFSIZE);

	/*defining filepath and filename where the fetched data will be stored*/
	strcat(home, "/");
	strcat(home, argv[3]);
/*	printf ("path = %s\n", home);*/
	file = fopen(home, "w+");

	total = 0;
	while ((bytes=read(sockfd, buff, BUFSIZE))>0){
		total += bytes;
		/*printf("total= %d\n", total);	*/

		/*deep copy of buffer, because strtok modifies it*/
		for (i=0; i<BUFSIZE; i++){
			buff_copy[i] = buff[i];	
			buff2_copy[i] = buff[i];	
		}
		/*checking if response contains 404 code and if the buffer is still processing headers, i.e. \r\n\r\n is not reached yet*/
		if (strstr(buff_copy, "404 Not Found") && double_newline == 0){
			printf("File NOT FOUND\n");
			break;
		}
		if (strstr(buff_copy, "200 OK") && double_newline == 0){
			printf("The request was successful. Returned code: 200\n");

		}
/*		printf("Received content: %s\n", buff);*/
		/*printf("Bytes received: %d\n", bytes);*/

		/*calculating the response headers size*/
		if (double_newline == 0){
			double_newline = strstr(buff_copy, "\r\n\r\n");
			if (double_newline){
				headers_length = (double_newline - buff_copy) + 4;
	/*			printf("Headers length= %d\n", headers_length);*/
			}
		}

		/*identifying content length*/
		if ((strtok(buff2_copy, "Content-Length:") != NULL) && (content_length == 0)){
			strtok(NULL, " ");
			content_length = strtol(strtok(NULL, "\r\n"), NULL, 10);
		/*	printf("Content length: %d\n", content_length);*/
		} 
		else if (content_length == 0) {
			continue;
	}		/*w+ means file will be overwritten if it exists*/				

		/*first iteration*/
		if (total == bytes){
			partial_content_size = bytes - headers_length;
			written_content_length = fwrite(buff + headers_length, 1, partial_content_size, file);
			if (written_content_length != partial_content_size){
				fprintf(stderr, "Error writing to file\n");
			}
		} else {
			written_content_length = fwrite(buff, 1, bytes, file);
		}
	
		if (total == (content_length + headers_length)){
		/*	printf("received:%d\n", total);*/
			break;
		}
				
	}
/*	printf("total received:%d \n", total);*/
	if (file){
		fclose(file);		
	}
	if (bytes < 0){
		perror("Receive error\n");
		return 1;
	}

	

 	return 0;	

}
