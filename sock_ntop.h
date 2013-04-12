#include <sys/socket.h>
#ifndef SOCK_NTOP_H_INCLUDED
#define SOCK_NTOP_H_INCLUDED
char *sock_ntop(const struct sockaddr *sa, socklen_t salen);
#endif 
char *http_response(int flag, int content_length, char *file_contents, size_t *response_size);
int handle_http(int clisockfd);
void send_status(int clisockfd, char *status, size_t status_size);
void send_response(int clisockfd, char *response, size_t response_size);
void sig_child(int signo);
int listening(char *hostname, char *service);

