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


/* DNS client */



char *query(char *host, size_t domain_length, char *name){

	char *q;
	char *query_str ="POST /dns-query HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %lu\r\nIam: oate1\r\n\r\nName=%s&Type=A";
	/*determining content length*/
	char *content = "Name=&Type=A";
	domain_length = domain_length + strlen(content);
	q = malloc(snprintf(NULL, 0, query_str, host, domain_length, name) + 1);
	snprintf(q, snprintf(NULL, 0, query_str, host, domain_length, name) + 1, query_str, host, domain_length, name);
/*	printf("the query: %s\n", q);*/
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
	int i;
	char buff[BUFSIZE], buff_copy[BUFSIZE];
	char *q;	
	/*char *home = getenv("HOME");*/
	char *double_newline=0;
	size_t size_domain; 
	struct timeval tv;

	tv.tv_sec = 15;
	tv.tv_usec = 0;
	
	if (argc != 4){
		fprintf(stderr, "usage: a.out IP_address/hostname port www.domain.com\n");
		return 1;
	}
	

	sockfd = connecting (argv[1], argv[2]);	
	
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	/*forming query to send to HTTP server*/
	size_domain = strlen(argv[3]);
	q = query(argv[1], size_domain, argv[3]);
	
	if (sockfd < 0){
		perror("Error creating socket\n");
		free(q);
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
		free(q);
		return 1;
	}
	free(q);

	bzero(buff, BUFSIZE);
	bzero(buff_copy, BUFSIZE);
	/*defining filepath and filename where the fetched data will be stored*/
/*	printf ("path = %s\n", home);*/

	total = 0;
	while ((bytes=read(sockfd, buff, BUFSIZE))>0){
		total += bytes;
		sleep(5);	
		/*	printf("total= %d\n", total);	*/

		/*deep copy of buffer, because strtok modifies it*/
		for (i=0; i<BUFSIZE; i++){
			buff_copy[i] = buff[i];	
		}
		/*checking if response contains 404 code and if the buffer is still processing headers, i.e. \r\n\r\n is not reached yet*/
		if (strstr(buff, "404 Not Found") && double_newline == 0){
			printf("Record for such a domain was not found\n");
			break;
		}
		if (strstr(buff, "200 OK") && double_newline == 0){
			printf("The request was successful. Returned code: 200\n");

		}

/*		printf("Received content: %s\n", buff);*/
		/*printf("Bytes received: %d\n", bytes);*/

		/*calculating the response headers size*/
		if (double_newline == 0){
			double_newline = strstr(buff, "\r\n\r\n");
			if (double_newline){
				headers_length = (double_newline - buff) + 4;
				printf("A records for requested domain name:\n");
				printf("%s", double_newline + 4);
/*				printf("Headers length= %d\n", headers_length);*/
			}
		} else {
			printf("%s", buff);	
		}

		/*identifying content length*/
		if ((strtok(buff_copy, "Content-Length:") != NULL) && (content_length == 0)){
			strtok(NULL, " ");
			content_length = strtol(strtok(NULL, "\r\n"), NULL, 10);
/*			printf("Content length: %d\n", content_length);*/
		} 
		else if (content_length == 0) {
			continue;
		}
		
	
		if (total == (content_length + headers_length)){
		/*	printf("received:%d\n", total);*/
		/*	printf("buffer received: %s\n", buff);*/
			

			break;
		}
				
	}
/*	printf("total received:%d \n", total);*/
	if (bytes < 0){
		if (errno == EWOULDBLOCK){
			fprintf(stderr, "Socket timeout\n");
			return 1;
		} else {
			perror("Receive error\n");
			return 1;
		}
	}
	if (bytes == 0 && total == 0){
		fprintf(stderr, "Nothing was sent by the server in response\n");
		
	}


 	return 0;	

}
