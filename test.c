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
	    char *name;
		unsigned short type;
		unsigned short class;
	};

int main(int argc, char **argv){

	char *hostname;
	char buff[10000];
	int bytes;
	int sockfd;
	struct DNS_query *q;
	char address[128];
	struct UDP_header *uheader;
	struct sockaddr_in *dest_address; 
	char ip_text[INET_ADDRSTRLEN];
	hostname = argv[1];	
	printf("entered hostname: %s\n", argv[1]);
	uheader = (struct UDP_header *) &buff;

	uheader->id = getpid();
	uheader->qr = 0;
	uheader->Opcode = 0;
	uheader->aa = 0;
	uheader->tc = 0;
	uheader->rd = 1;
	uheader->ra = 0;
	uheader->z = 0;
	uheader->ad = 0;
	uheader->cd = 0;
	uheader->rcode = 0;
	uheader->qcount = htons(1);
	uheader->acount = 0;
	uheader->nscount = 0;
	uheader->arcount = 0;
	/*create a UDP socket*/
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0){
		perror("error creating socket\n");
	}
	bzero(dest_address, sizeof(dest_address));
	dest_address = (struct sockaddr_in *) &address;
	dest_address->sin_family = AF_INET;
	dest_address->sin_port = htons(53);
	dest_address->sin_addr.s_addr = inet_addr("8.8.8.8");

/*	printf("sin_port: %u\n", dest_address->sin_port);	*/
	inet_ntop(AF_INET, &(dest_address->sin_addr), ip_text, INET_ADDRSTRLEN);	

	q = (struct DNS_query *) &buff[sizeof(struct UDP_header) + 1];
	memcpy(q, hostname, strlen(hostname) + 1);
	printf("copied stuff: %s\n", q->name);
/*	printf("ip_address: %s\n", ip_text);*/
	bytes = sendto(sockfd, &buff, sizeof(struct UDP_header) + sizeof(struct DNS_query), 0, (struct sockaddr *) dest_address, sizeof(struct sockaddr_in));
	
	/*processName(&uheader, hostname);*/
	return 0;
}


/* Processes one string in a DNS resource record.
 *
 * bstart (in):	pointer to the start of the DNS message (UDP payload)
 * bcur (in):	pointer to the currently processed position in a message.
		This should point to the start of compressed or uncompressed
		name string.
 * name (out):	buffer for storing the name string in dot-separated format
 * "www.google.com" = name
 * returns:	updated position of bcur, pointing to the next position
 *		following the name 
 */
uint8_t *processName(uint8_t *bstart, uint8_t *bcur, char *name)
{
	uint8_t *p = bcur;
	char strbuf[80];
	char *strp;
	int compressed = 0;
	name[0] = 0;

	do {
		strp = strbuf;

		if ((*p & 0xc0) == 0xc0) { /* first two bits are set =>
					    compressed format */
			uint16_t offset = (*p & 0x3f);
			offset = (offset << 8) + *(p+1);

			p = bstart + offset; /* move the read pointer to the
						offset given in message */

			if (!compressed) bcur += 2; /* adjustment of bcur must
						       only be done once, in
						       case there are multiple
						       nested pointers in msg */
			compressed = 1;
		} else if (*p > 0) {
			/* strbuf contains one element of name, not full name*/
			memcpy(strbuf, p+1, *p);
			strp += *p;
			p += *p + 1;

			if (!compressed)
				bcur = p; /* adjustment of bcur based on string
					     string length is only done if it
					     was not compressed, otherwise it
					     is assumed to be 16 bits always */
			*strp = '.';
			*(strp+1) = 0;
			strcat(name, strbuf);
		}
	} while (*p > 0);

	if (!compressed) bcur++; /* compensate for trailing 0 (unless name was
				    compressed) */
	return bcur;
}
