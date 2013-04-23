server: sock_ntop.c webserver.c
	gcc -o s -g -Wall -pedantic sock_ntop.c webserver.c

client: webclient.c
	gcc -Wall -g -pedantic webclient.c

upload: uploadclient.c
	gcc -o u -g -Wall -pedantic uploadclient.c

dnsclient: dnsclient.c
	gcc -o d -g -Wall -pedantic dnsclient.c

test: test2.c 
	gcc -o t -g -Wall test2.c
