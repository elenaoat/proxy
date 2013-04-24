#include "headers.h"
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

#define MAX_DATA_LENGTH 1000
uint8_t *processName(uint8_t *bstart, uint8_t *bcur, char *name);
void name_encode(char* name, char* name_encoded);
struct DNS_header {
    uint16_t id; /* identification number */

   	uint8_t rd :1; /* recursion desired */
    uint8_t tc:1; /* truncated message*/
    uint8_t aa :1; /* authoritive answer*/
    uint8_t opcode :4; /* purpose of message*/
    uint8_t qr :1; /* query/response flag*/

    uint8_t rcode :4; /* response code*/
    uint8_t cd :1; /* checking disabled*/
    uint8_t ad :1; /* authenticated data*/
    uint8_t z :1; /* its z! reserved*/
    uint8_t ra :1; /* recursion available*/

 
    uint16_t q_count; /* number of question entries*/
    uint16_t ans_count; /* number of answer entries*/
    uint16_t auth_count; /* number of authority entries*/
    uint16_t add_count; /* number of resource entries*/
};


struct DNS_query {
	uint16_t type;
	uint16_t class;
};

typedef struct {
	char *name;
	struct DNS_query *q; 		
} QUERY;


struct RESPONSE_fields {
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t dl;
};

struct RESPONSE{
	char *name;
	struct RESPONSE_fields *rf;
	char *res_data;
};

#define BUF_SIZE 65536
#define NAME_SIZE 100
char* dns_query(char *domain, int query_type, size_t *response_to_client_size){
	char *response_to_client, *text;
	int flag = 1;
	char name_dotted[NAME_SIZE];
	char buff[BUF_SIZE], *name, buff_rec[BUF_SIZE];
	uint8_t *pointer;
	struct RESPONSE_fields *res_fields;
	struct RESPONSE *answer;
	struct DNS_query *question;
	int i, j, bytes, bytes_rec;
	int sockfd;
	struct DNS_header *uheader;
	struct sockaddr_in dest_address; 
	char ip_text[INET_ADDRSTRLEN];
	struct timeval tv;
	/*printf("entered hostname: %s\n", argv[1]);*/
/*	bzero(buff, 65536);*/
	tv.tv_sec = 10;
    tv.tv_usec = 0;

	
	uheader = (struct DNS_header *) buff;

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
	/*set socket flags for timeout*/
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	
	if(sockfd < 0){
		perror("error creating socket\n");
	}
/*	dest_address = (struct sockaddr_in *) &address;*/
	bzero(&dest_address, sizeof(dest_address));
	dest_address.sin_family = AF_INET;
	dest_address.sin_port = htons(53);
	dest_address.sin_addr.s_addr = inet_addr("8.8.8.8");
	/*dest_address.sin_addr.s_addr = inet_addr("208.67.222.222");*/
/*	printf("sin_port: %u\n", dest_address->sin_port);	*/
	inet_ntop(AF_INET, &(dest_address.sin_addr), ip_text, INET_ADDRSTRLEN);	
	/*!!!*/
	name = &buff[sizeof(struct DNS_header)];

	name_encode(domain, name);
	question = (struct DNS_query *) &buff[sizeof(struct DNS_header) + strlen((const char *) name) + 1];
	/*query type*/
	question->type = htons((short) query_type);
	question->class = htons(1);
/*	printf("%s\n", name);*/
/*	printf("%d\n", question->type);*/
	/*printf("sockfd: %i\n", sockfd);*/
/*	printf("name: %s\n", name);*/
	bytes = sendto(sockfd, buff, sizeof(struct DNS_header) + sizeof(struct DNS_query) + strlen((const char *) name) + 1, 0, (struct sockaddr *) &dest_address, sizeof(dest_address));
	printf("bytes sent to DNS server: %d\n", bytes);	
	if (bytes == -1){
		perror("sendto error\n");
		return "\0";
	}

	bzero(buff_rec, BUF_SIZE);
	
	bytes_rec = recvfrom(sockfd, &buff_rec, BUF_SIZE, 0, NULL, 0);
	printf("bytes received: %d\n", bytes_rec);
	if (bytes_rec < 0){
	/*char *response_to_client;*/
		perror("recvfrom:\n");	
		if (errno == EWOULDBLOCK){
			fprintf(stderr, "UDP socket timeout. UDP packet probably lost\n");
			flag = 0;
			text =  "UPD packet lost. Try again";
			response_to_client = http_response(flag, strlen(text), text, response_to_client_size);
		/*	printf("%s\n", response_to_client);*/
			return response_to_client;		
		} else {
			perror("receive error\n");	
		}
	}
		

	printf("bytes received from DNS server in response: %d\n", bytes_rec);
	uint16_t que_num, ans_num, auth_num, add_num;
	uint8_t response_code, qr;
	uheader = (struct DNS_header *) buff_rec;
	response_code = uheader->rcode;
	if (response_code != 0){
		flag = 0;
		switch (response_code){
		case 1:
			text =  "Incorrect DNS format of supplied name";
			break;
		case 2:
			text =  "There was a server failure. Try again later";
			break;
		case 3:
			text = "The name supplied does not exist";
			break;
		case 4:
			text = "Following query is not supported";
			break;
		case 5:
			text = "Refused to resolve the name";
			break;
		}
		response_to_client = http_response(flag, strlen(text), text, response_to_client_size);
		return response_to_client;		
	}

	qr = uheader->qr;
	que_num = ntohs(uheader->q_count);
	ans_num = ntohs(uheader->ans_count);
	if (ans_num == 0){
		text = "No records found";
		flag = 0;
		response_to_client = http_response(flag, strlen(text), text, response_to_client_size);
		return response_to_client;		
	}
	auth_num = ntohs(uheader->auth_count);
	add_num = ntohs(uheader->add_count);
	printf("response question number: %d\nresponse answer number: %d\n", que_num, ans_num);
	printf("response code: %d, query/response: %d\n", response_code, qr);
	printf("response authoritative records number: %d\nresponse additional records number: %d\n", auth_num, add_num);
	answer = calloc(ans_num, sizeof(struct RESPONSE));


	/*point to part after DNS header*/
	pointer = (unsigned char *) &buff_rec[(sizeof(struct DNS_header) + strlen((const char *) name) + 1 + sizeof(struct DNS_query))];

		struct RESPONSE_fields rf;
		const char *ret_val;
		i=0;
		int size; 		
		char response_parsed[NAME_SIZE][MAX_DATA_LENGTH];
		bzero(response_parsed, NAME_SIZE*MAX_DATA_LENGTH);
	/*processing answers section*/
	for (j=0; j<ans_num; j++){	
		bzero(name_dotted, NAME_SIZE);
		pointer = processName ((unsigned char*) buff_rec, pointer, name_dotted);
		answer[j].name = name_dotted;

/*		printf("response name: %s\n", name_dotted);*/
		/*point to response fields structure: type, class, TTL, data length */
		res_fields = (struct RESPONSE_fields *) pointer;

		rf.type = ntohs(res_fields->type);	
		rf.class = ntohs(res_fields->class);
		rf.ttl = ntohs(res_fields->ttl);
		rf.dl = ntohs(res_fields->dl);
		size = rf.dl;
		char address[MAX_DATA_LENGTH];
		answer[j].res_data = address;
		answer[j].rf = &rf;
/*		printf("response type: %d\nresponse class: %d\nresponse TTL: %d\nresponse data length: %d\n", type, class, TTL, resource_data_length);*/

		/*point to the beginning of resource data, 10 = sizeof RESPONSE_length*/
		pointer = pointer + 10;

		struct in_addr * ipv4_addr;
		struct in6_addr * ipv6_addr;
		switch (rf.type) {
			case 1: // A
				ipv4_addr = (struct in_addr *) pointer;
				ret_val = inet_ntop(AF_INET, ipv4_addr, answer[j].res_data , INET_ADDRSTRLEN);
				break;
			case 28:
				ipv6_addr = (struct in6_addr *) pointer;
				ret_val = inet_ntop(AF_INET6, ipv6_addr, answer[j].res_data , INET6_ADDRSTRLEN);
				break;
		}
		if (ret_val <= 0){
			perror("error converting an IP address\n");
			flag = 0;
			/*TODO follow http_response*/
			break;
		} 
		else {
	/*		printf("response ip address: %s\n", answer[j].res_data);*/
			/*copy the IP address with the null byte*/

			/*case when the response is */
			strcpy(response_parsed[j], answer[j].res_data);
		/*	printf("ip address: %s", response_parsed[j]);*/
		}
		pointer = pointer + answer[j].rf->dl;
	}
	
	char unidim_response[MAX_DATA_LENGTH*ans_num];
	bzero(unidim_response, MAX_DATA_LENGTH*ans_num);
	int k=0;
	/*convert 2-d answers section into 1-d answers section*/
	for (j=0; j<ans_num; j++){
		for (i=0; i<strlen(response_parsed[j]); i++){
			unidim_response[k] = response_parsed[j][i];
		/*	printf("%c", unidim_response[k]);*/
			k++;
		}
		unidim_response[k] = '\n';
		k++;
	/*	printf("\n");*/
	}
/*	for (j=0; j<ans_num; j++){
		printf("%s\n", unidim_response);
	}*/
/*	printf("size of content in dnsquery.c: %d\n", ans_num*INET_ADDRSTRLEN);*/
	response_to_client = http_response(flag, k, unidim_response, response_to_client_size);		
	/*printf("response_to_client: %s\n", response_to_client);*/
	/*free(response_to_client);*/
	free(answer);
	close(sockfd);	
	return response_to_client;
}

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
/* function that does conversion www.gmail.com -> */
void name_encode(char* name, char* name_encoded){
    char *ptr;
    int diff, i;
    ptr = strchr(name, '.');
	/*if (ptr == NULL) {
		printf("no dot found\n");
		name_encoded[0]=strlen(name);
		for (i=1; i<strlen(name); i++){
			name_encoded[i] = name[i-1]; 
		}
		name_encoded[i] = 0;
		printf("name encoded: %s\n", name_encoded);
	} else {*/
    diff = ptr - name;
    name_encoded[0] = diff;
    
    for (i=0; i<strlen(name); i++){
        name_encoded[i+1] = name[i];
        if (name[i] == '.'){
            ptr = strchr(&name[i+1], '.');
            if (ptr == NULL){
                ptr = strchr(&name[i], '\0');
            }
            diff = ptr - &name[i] - 1;      
            name_encoded[i+1] = diff;
        }
    }
    name_encoded[i+1] = 0;

	/*}*/
    /*printf ("%s, strlen=%lu\n", name_encoded, strlen(name_encoded));*/
}
/*	printf("The number of questions: %d\n", ntohs(uheader->q_count));
	printf("The number of answers: %d\n", ntohs(uheader->ans_count));	*/

/*	printf("sizeof(unsigned int) = %lu sizeof(unsigned short) = %lu\n", sizeof(unsigned int), sizeof(unsigned short));	
	
	printf("sizeof(unsigned char) = %lu sizeof(char) = %lu\n", sizeof(unsigned char), sizeof(char));	
*/

/*	printf("sizeof(uint8_t)=%lu, sizeof(char)=%lu\n", sizeof(uint8_t), sizeof(char));*/


/* Processes one string in a DNS resource record.
 *
 * bstart (in):	pointer to the start of the DNS message (UDP payload)
 * bcur (in):	pointer to the currently processed position in a message.
		This should point to the start of compressed or uncompressed
		name string.
 * name (out):	buffer for storing the name string in dot-separated format
 *
 * returns:	updated position of bcur, pointing to the next position
 *		following the name 
 */
/* TODO
 * add malloc into the loop and assign to each var values that are in order to save them into another array
 * case when the response cannot 
 *
 *
 *
 * */
