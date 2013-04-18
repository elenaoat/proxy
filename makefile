server: sock_ntop.c webserver.c
	gcc -o s -Wall -pedantic sock_ntop.c webserver.c

client: webclient.c
	gcc -Wall -pedantic webclient.c

upload: uploadclient.c
	gcc -o u -Wall -pedantic uploadclient.c


test: test1.c 
	gcc -o t -Wall test1.c
