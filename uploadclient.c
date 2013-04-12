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
#define BUFSIZE 1000
#include <stddef.h>
#define MAXPATH 1024
char *query(char *host, char *resource, int content_length, char *file_contents, size_t *request_length){
	char content_length_str[10];
	char *q, *request=0;
	int size_headers; 
	char *query_str ="PUT /%s HTTP/1.1\r\nHost: %s\r\nContent-Type:\
	text/plain\r\nContent-Length: %s\r\nIam: oate1\r\n\r\n";
	/*casting the int value of content length to string*/
	snprintf(content_length_str, 10, "%d", content_length);
	size_headers = snprintf(NULL, 0, query_str, resource, host, content_length_str);
	/*printf("size headears: %d\n", size_headers);	*/
	*request_length = size_headers + content_length;
	/*printf("request length:%lu\n", *request_length);*/
	q = malloc(*request_length);
	snprintf(q, *request_length,  query_str, resource, host, content_length_str);
	/*printf("the query: %s\n", q);*/

	request = calloc(*request_length, sizeof(char));
	memcpy(request, q, size_headers);
	memcpy(request + size_headers, file_contents, content_length);
/*	printf("request:\n%s\n", request);*/
	free(q);
	return request;
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
	
	int sockfd, total=0, bytes;
	int length;
	char buff[BUFSIZE];/*, buff_copy[BUFSIZE];*/
	FILE *file;
	char *q, *file_contents;
	size_t request_length;
	size_t *request_length_ptr = &request_length;	
	char home[MAXPATH];
	/*	char *double_newline=0;
	int partial_content_size;*/

	getcwd(home, MAXPATH);
	if (argc != 4){
		fprintf(stderr, "usage: a.out IP_address/hostname port filename\n");
		return 1;
	}
	
	sockfd = connecting (argv[1], argv[2]);	
	
	/*forming query to send to HTTP server*/
	
	if (sockfd < 0){
		perror("Error creating socket\n");
		return 1;
	}	

	/*defining filepath and filename where the uploaded data is stored*/	
	strcat(home, "/");
	strcat(home, argv[3]);
	/*printf("path: %s\n", home);*/
	
	/*w+ means file will be overwritten if it exists*/
	file = fopen(home, "r");

	/*identifying upload file content length*/
    if (file){
    	fseek(file, 0, SEEK_END);
        length = ftell(file) ;
        fseek(file, 0, SEEK_SET);
/*        file_contents = malloc(length + 1);
        bzero(file_contents, length + 1);*/
	/*	printf("file length: %d\n", length);*/

		/*file_contents: contents of file, char array, not a string, does not contain null byte*/
		file_contents = malloc(length);
		bzero(file_contents, length);
        if (file_contents){
        	/*reading file contents into file_contents char array*/
            fread(file_contents, 1, length, file);
            /*file_contents[length] = 0;*/
        }

	} else {
		fprintf(stderr, "File not found\n");
		exit(1);
	}

	q = query(argv[1], argv[3], length, file_contents, request_length_ptr);

/*	request_length = request_length - 1;*/
/*	FILE *testing;
	testing = fopen("/home/elena/school/NetProgramming/proxy/contents.txt", "w+"); */
	/*send request to server, request_length-1: because we don't need to send null byte*/
   	while ((bytes=write(sockfd, q, request_length)) > 0){
/*		fwrite(q, request_length, 1, testing);*/
		total += bytes;
		if (total == request_length){
			break;
		}	
	}
/*	fclose(testing);*/
	if (bytes < 0){
		perror("Send error. Possibly the port or hostname are wrong.\n");
		return 1;
	}
	bytes = 0;
	while ((bytes=read(sockfd, buff, BUFSIZE)) > 0){
		printf("The status of the response received from http server: %s\n", buff);	
	}
	
	
	close(sockfd);
	free(q);
 	return 0;	

}
