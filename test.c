#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

	struct UDP_header{
		unsigned int id : 16;
		
		unsigned int qr : 1; /*query or response*/
		unsigned int Opcode : 4;/**/
		unsigned int aa : 1; /**/
		unsigned int tc : 1; /**/
		unsigned int rd : 1;/**/
		unsigned int ra : 1;/**/
		unsigned int z : 1;/**/
		unsigned int ad : 1;/**/
		unsigned int cd : 1;/**/
		unsigned int rcode : 4;/**/
		
		unsigned int qcount;/*question count*/
		unsigned int acount;/*answer count*/
		unsigned int nscount;/*authority count*/
		unsigned int arcount;/*addition info count*/		
	};

	struct DNS_query{
		unsigned char *name;
		unsigned short type;
		unsigned short class;
	};

int main(int argc, char **argv){

	char *p;
	char * hostname;
	int bytes;
	int sockfd;
	struct UDP_header uheader;
	struct sockaddr_in dest;
	struct sockaddr_in *dest_address = &dest; 
	char ip_text[INET_ADDRSTRLEN];
	hostname = argv[1];	
	printf("entered hostname:%s\n", argv[1]);

	uheader.id = getpid();
	uheader.qr = 0;
	uheader.Opcode = 0;
	uheader.aa = 0;
	uheader.tc = 0;
	uheader.rd = 1;
	uheader.
	/*create a UDP socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0){
		perror("error creating socket\n");
	}
	bzero(dest_address, sizeof(dest_address));
	dest_address->sin_family = AF_INET;
	dest_address->sin_port = htons(53);
	dest_address->sin_addr.s_addr = inet_addr("8.8.8.8");
/*	printf("sin_port: %u\n", dest_address->sin_port);	*/
	inet_ntop(AF_INET, &(dest_address->sin_addr), ip_text, INET_ADDRSTRLEN);	
/*	printf("ip_address: %s\n", ip_text);*/
	bytes = sendto(sockfd, sizeof(struct UDP_header) + sizeof(DNS_query), dest_address, sizeof(struct sockaddr_in));
	return 0;	
}
