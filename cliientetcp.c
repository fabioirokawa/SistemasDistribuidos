#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>

#define DATA "Esta eh a mensagem que quero enviar"

main(argc, argv)
     int argc;
     char *argv[];
{
	int sock;
	struct sockaddr_in server;
	struct hostent *hp, *gethostbyname();
	char buf[1024];

        /* Cria o socket de comunicacao */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0) {
	/*
	/- houve erro na abertura do socket
	*/
		perror("opening stream socket");
		exit(1);
	}

	/* Associa */
	server.sin_family=AF_INET;
        hp = gethostbyname(argv[1]);
        if (hp==0) {
            fprintf(stderr, "%s: unknown host ", argv[1]);
            exit(2);
        }
        bcopy ((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[2]));

	if (connect (sock,(struct sockaddr *)&server,sizeof server)<0) {
                perror("connecting stream socket");
		exit(1);
	}
	if (write(sock, DATA, sizeof DATA)<0)
		perror("writing on stream socket");
        close(sock);
        exit(0);
}


