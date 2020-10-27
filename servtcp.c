#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define TRUE 1

int main(void)
{
	int sock, length;
	struct sockaddr_in server;
	int msgsock;
	char buf[1024];
	int rval, i;

	/* Cria o socket */
	sock=socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		perror("opening stream socket");
		exit(1);
	}
	
	/* Associa */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port=0;
	if (bind(sock,(struct sockaddr *) &server, sizeof server) < 0) {
		perror("binding stream socket");
		exit(1);
	}

	/* Imprime o numero da porta */
	length=sizeof server;
	if (getsockname(sock, (struct sockaddr *) &server, &length)<0) {
		perror("getting socket name");
		exit(1);
	}
	printf("Socket port #%d\n",ntohs(server.sin_port));
	
	/* Conexoes */
	listen(sock,5);
	do {
		msgsock=accept(sock,(struct sockaddr *) 0, (int *) 0);
		if (msgsock==-1)
			perror("accept");
		else do {
			bzero(buf, sizeof buf);
			if ((rval=read(msgsock,buf,1024))<0)
				perror("reading stream message");
			i=0;
			if (rval==0)
				printf("Ending connection\n");
			else printf("-->%s\n",buf);
		     } while (rval!=0);
		close (msgsock);
	} while (TRUE);

	exit(0);
}
	
