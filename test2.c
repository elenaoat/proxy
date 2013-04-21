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
void name_encode(char* name, char* name_encoded);
char *http_response(int flag, int content_length, char *file_contents, size_t *response_size_total){

    char *status;
    char content_length_str[10];
    size_t size_response;
    char *response = "HTTP/1.1 %s\r\nIam: oate1\r\nContent-Length: %s\r\nContent-Type: text/plain\r\n\r\n";
    char *r=0, *response1;

    if (flag == 1)
         status = "200 OK";
    else
        status = "404 Not Found";

    /*casting the int value of content length to string*/
    snprintf(content_length_str, 10, "%d", content_length);

    /*calculating size of http headers and concatenating them*/
    size_response = snprintf(NULL, 0, response, status, content_length_str);
    response1 = calloc(size_response + 1, sizeof(char));
/*  printf("size of headers: %lu\n", size_response);*/
    snprintf(response1, size_response + 1, response, status, content_length_str);
    /*calculating size of headers + content and forming whole HTTP response*/
    *response_size_total = size_response + content_length + 1;
    r = calloc(*response_size_total, sizeof(char));
/*  printf("size of whole response: %lu\n", *response_size_total);*/

    /*putting together headers and binary content*/
    memcpy(r, response1, size_response);
    memcpy(r + size_response, file_contents, content_length);

    /*freeing occupied memory*/
    free(response1);
    return r;


}

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
int main(int argc, char **argv){
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
	/*printf("entered hostname: %s\n", argv[1]);*/
/*	bzero(buff, 65536);*/
	
	
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
	if(sockfd < 0){
		perror("error creating socket\n");
	}
/*	dest_address = (struct sockaddr_in *) &address;*/
	bzero(&dest_address, sizeof(dest_address));
	dest_address.sin_family = AF_INET;
	dest_address.sin_port = htons(53);
	dest_address.sin_addr.s_addr = inet_addr("8.8.8.8");
	/*dest_address.sin_addr.s_addr = inet_addr("208.67.222.222");*/
	if (argc != 2){
		printf("the name wasn't entered\n");
	} else {
/*		printf("%s\n", argv[1]);*/
	}
/*	printf("sin_port: %u\n", dest_address->sin_port);	*/
	inet_ntop(AF_INET, &(dest_address.sin_addr), ip_text, INET_ADDRSTRLEN);	
	/*!!!*/
	name = &buff[sizeof(struct DNS_header)];

	name_encode(argv[1], name);
	question = (struct DNS_query *) &buff[sizeof(struct DNS_header) + strlen((const char *) name) + 1];
	/*query type*/
	question->type = htons(1);
	question->class = htons(1);
/*	printf("%s\n", name);*/
/*	printf("%d\n", question->type);*/
	/*printf("sockfd: %i\n", sockfd);*/
/*	printf("name: %s\n", name);*/
	bytes = sendto(sockfd, buff, sizeof(struct DNS_header) + sizeof(struct DNS_query) + strlen((const char *) name) + 1, 0, (struct sockaddr *) &dest_address, sizeof(dest_address));
	printf("bytes sent: %d\n", bytes);	
	if (bytes < -1){
		perror("sendto error\n");
		return -1;
	}

	bzero(buff_rec, BUF_SIZE);
	
	bytes_rec = recvfrom(sockfd, &buff_rec, BUF_SIZE, 0, NULL, 0);
	if (bytes_rec < 0){
		perror("receive error\n");
	}
	printf("bytes sent back: %d\n", bytes_rec);
	uint16_t que_num, ans_num, auth_num, add_num;

	uheader = (struct DNS_header *) buff_rec;
	que_num = ntohs(uheader->q_count);
	ans_num = ntohs(uheader->ans_count);
	auth_num = ntohs(uheader->auth_count);
	add_num = ntohs(uheader->add_count);
	printf("response question number: %d\nresponse answer number: %d\n", que_num, ans_num);
	printf("response authoritative records number: %d\nresponse additional records number: %d\n", auth_num, add_num);
	answer = calloc(sizeof(struct RESPONSE), ans_num);


	/*point to part after DNS header*/
	pointer = (unsigned char *) &buff_rec[(sizeof(struct DNS_header) + strlen((const char *) name) + 1 + sizeof(struct DNS_query))];

		struct RESPONSE_fields rf;
		const char *ret_val;
		struct sockaddr_in a;
		long *p;
		i=0;
		int size; 		
		char response_parsed[NAME_SIZE][INET_ADDRSTRLEN];
		bzero(response_parsed, NAME_SIZE*INET_ADDRSTRLEN);
	/*processing answers section*/
	for (j=0; j<ans_num; j++){	
		bzero(name_dotted, NAME_SIZE);
		pointer = processName ((unsigned char*) buff_rec, pointer, name_dotted);
		answer[j].name = name_dotted;

		printf("response name: %s\n", name_dotted);
		/*point to response fields structure: type, class, TTL, data length */
		res_fields = (struct RESPONSE_fields *) pointer;

		rf.type = ntohs(res_fields->type);	
		rf.class = ntohs(res_fields->class);
		rf.ttl = ntohs(res_fields->ttl);
		rf.dl = ntohs(res_fields->dl);
		size = rf.dl;
		char address[size];
		answer[j].res_data = address;
		answer[j].rf = &rf;
/*		printf("response type: %d\nresponse class: %d\nresponse TTL: %d\nresponse data length: %d\n", type, class, TTL, resource_data_length);*/

		/*point to the beginning of resource data, 10 = sizeof RESPONSE_length*/
		pointer = pointer + 10;
		p = (long*) pointer;
		a.sin_addr.s_addr = (*p);

		/*for (i=0; i<size; i++){
			response_parsed[i]  
			
		}*/
		/*inet_ntop null terminates the string*/
		ret_val = inet_ntop(AF_INET, &(a.sin_addr), answer[j].res_data , INET_ADDRSTRLEN);
	
		if (ret_val <= 0){
			perror("error converting an IP address\n");
			flag = 0;
			break;
		} 
		else {
			printf("response ip address: %s\n", answer[j].res_data);
			/*copy the IP address with the null byte*/

			/*case when the response is */
			for (i=0; i<INET_ADDRSTRLEN ; i++){
				response_parsed[j][i] = answer[j].res_data[i];
			}

		}
		pointer = pointer + answer[j].rf->dl;
	}
	
	char *client_response;
	size_t client_response_size;
	char unidim_response[INET_ADDRSTRLEN*ans_num];
	int k=0;
	/*convert 2-d answers section into 1-d answers section*/
	for (j=0; j<ans_num; j++){
		for (i=0; i<INET_ADDRSTRLEN; i++){
			unidim_response[k] = response_parsed[j][i];
			k++;
		}
	}
	
	client_response = http_response(flag, ans_num*INET_ADDRSTRLEN, unidim_response, &client_response_size);		
	printf("client_response: %s\n", client_response);
	free(client_response);
	free(answer);
	close(sockfd);	
	return 0;
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
    diff = ptr - name;
    name_encoded[0] = diff;
    
    for (i=0; i<strlen(name); i++){
        name_encoded[i+1] = name[i];
        if (name[i] == '.'){
            ptr = strchr(&name[i+1], '.');
            if (ptr == NULL){
                printf("ptr null\n");
                ptr = strchr(&name[i], '\0');
            }
            diff = ptr - &name[i] - 1;      
            name_encoded[i+1] = diff;
        }
    }
    name_encoded[i+1] = 0;
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
