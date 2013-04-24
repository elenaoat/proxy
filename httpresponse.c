#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


char *http_response(int flag, int content_length, char *file_contents, size_t *response_size_total){

    char *status;
    size_t size_response;
    char *response = "HTTP/1.1 %s\r\nIam: oate1\r\nContent-Length: %d\r\nContent-Type: text/plain\r\n\r\n";
    char *r, *response1;

    if (flag == 1)
         status = "200 OK";
    else
        status = "404 Not Found";

    /*calculating size of http headers and concatenating them*/
    size_response = snprintf(NULL, 0, response, status, content_length);
    response1 = calloc(size_response + 1, sizeof(char));
/*  printf("size of headers: %lu\n", size_response);*/
    snprintf(response1, size_response + 1, response, status, content_length);
    /*calculating size of headers + content and forming whole HTTP response*/
    *response_size_total = size_response + content_length;
    r = calloc(*response_size_total, sizeof(char));
/*  printf("size of whole response: %lu\n", *response_size_total);*/

    /*putting together headers and binary content*/
    memcpy(r, response1, size_response);
    memcpy(r + size_response, file_contents, content_length);

    /*freeing occupied memory*/
    free(response1);
    return r;

}

