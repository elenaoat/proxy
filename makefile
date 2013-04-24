server: sock_ntop.c dnsquery.c httpresponse.c webserver.c
	gcc -o s -g -Wall dnsquery.c httpresponse.c sock_ntop.c webserver.c

client: webclient.c
	gcc -Wall -g -pedantic webclient.c

upload: uploadclient.c
	gcc -o u -g -Wall -pedantic uploadclient.c

dnsclient: dnsclient.c
	gcc -o d -g -Wall -pedantic dnsclient.c

