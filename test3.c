
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

int main(){
	
	char buff[500];
	name_encode("www.mail.ru.gggggg.fff", buff);
	return 0;
}









