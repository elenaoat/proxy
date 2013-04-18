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
uint8_t *processName(uint8_t *bstart, uint8_t *bcur, char *name);
void dns_format(unsigned char* dns, unsigned char* host);



struct UDP_header
{
    unsigned short id; // identification number

 
   	unsigned char rd :1; // recursion desired 
    unsigned char tc:1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag
 
    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available
 
    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};


/*struct UDP_header{
		unsigned int id : 16;
		
		unsigned int qr : 1; *//*query or response*/
/*		unsigned int Opcode : 4;
		unsigned int aa : 1; 
		unsigned int tc : 1; 
		unsigned int rd : 1;
		unsigned int ra : 1;
		unsigned int z : 1;
		unsigned int ad : 1;
		unsigned int cd : 1;
		unsigned int rcode : 4;
		
		unsigned int qcount;
		unsigned int acount;
		unsigned int nscount;
		unsigned int arcount;
	};*/

	struct DNS_query{
	   unsigned short type;
	   unsigned short class;
	};

typedef struct{
	unsigned char *name;
	struct DNS_query *q; 		
}QUERY;


struct response_fields{
	unsigned short type;
	unsigned short class;
	unsigned short ttl;
	unsigned short dl;
};

struct response{
	unsigned char *name;
	struct response_fields *rf;
	unsigned char *res_data;
};

int main(int argc, char **argv){

	unsigned char buff[65536], *name, buff_rec[65536], *reader;
	struct DNS_query *question;
	int bytes, bytes_rec;
	int sockfd;
	struct UDP_header *uheader;
	struct response *answer;	
	struct sockaddr_in dest_address; 
	char ip_text[INET_ADDRSTRLEN];
	/*printf("entered hostname: %s\n", argv[1]);*/
/*	bzero(buff, 65536);*/
	
	
	uheader = (struct UDP_header *) buff;

	uheader->id = getpid();
	uheader->qr = 0;
	uheader->opcode = 0;
	uheader->aa = 0;
	uheader->tc = 0;
	uheader->rd = 1; /*query is recursive*/
	uheader->ra = 0;
	uheader->z = 0;
	uheader->ad = 0;
	uheader->cd = 0;
	uheader->rcode = 0;
	uheader->q_count = htons(1);
	uheader->ans_count = 0;
	uheader->auth_count = 0;
	uheader->add_count = 0;
	/*create a UDP socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0){
		perror("error creating socket\n");
	}
/*	dest_address = (struct sockaddr_in *) &address;*/
	bzero(&dest_address, sizeof(dest_address));
	dest_address.sin_family = AF_INET;
	dest_address.sin_port = htons(53);
	dest_address.sin_addr.s_addr = inet_addr("208.67.222.222");
	if (argc != 2){
		printf("the name wasn't entered\n");
	} else {
		printf("%s\n", argv[1]);
	}
/*	printf("sin_port: %u\n", dest_address->sin_port);	*/
	inet_ntop(AF_INET, &(dest_address.sin_addr), ip_text, INET_ADDRSTRLEN);	
	/*!!!*/
	name = &buff[sizeof(struct UDP_header)];

	/*processName(buff, q, argv[1]);*/
	dns_format(name, (unsigned char *)  argv[1]);
	question = (struct DNS_query *) &buff[sizeof(struct UDP_header) + strlen((const char *) name) + 1];
	/*query type*/
	question->type = htons(1);
	question->class = htons(1);
/*	printf("%s\n", name);*/
/*	printf("%d\n", question->type);*/
	/*printf("sockfd: %i\n", sockfd);*/
/*	printf("name: %s\n", name);*/
	bytes = sendto(sockfd, buff, sizeof(struct UDP_header) + sizeof(struct DNS_query) + strlen((const char *) name) + 1, 0, (struct sockaddr *) &dest_address, sizeof(dest_address));
	printf("bytes sent: %d\n", bytes);	
	/*processName(&uheader, hostname);*/

	if (bytes < -1){
		perror("sendto error\n");
		return -1;
	}
	bzero(buff_rec, 65536);
	bytes_rec = recvfrom(sockfd, &buff_rec, 65536, 0, NULL, 0);
	if (bytes_rec < 0){
		perror("receive error\n");
	}
	printf("bytes in response: %d\n", bytes_rec);

	uheader = (struct UDP_header *) buff_rec;
	reader = &buff_rec[(sizeof(struct UDP_header) + strlen((const char *) name) + 1 + sizeof(struct DNS_query))];
	printf("The number of questions: %d\n", ntohs(uheader->q_count));
	printf("The number of answers: %d\n", ntohs(uheader->ans_count));	

	printf("sizeof(unsigned int) = %lu sizeof(unsigned short) = %lu\n", sizeof(unsigned int), sizeof(unsigned short));	
	
	printf("sizeof(unsigned char) = %lu sizeof(char) = %lu\n", sizeof(unsigned char), sizeof(char));	
	
	close(sockfd);	
	
	return 0;
}

void name_encode(char *name, char *name_encoded){
	char *ptr;

	strchr(name, '.');

}

void dns_format(unsigned char* dns, unsigned char* host)
{
    int lock=0 , i;
 
    strcat((char*)host,".");
 
    for(i=0 ; i<(int)strlen((char*)host) ; i++)
    {
        if(host[i]=='.')
        {
            *dns++=i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host[lock];
            }
            lock++; /*or lock=i+1;*/
        }
    }
    *dns++='\0';
}

