#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

#define IP_SERVER_DATA "localhost"
#define NOME_SEMAFORO "/semaforo1"
#define PORTA_SERVER_DATA1 5000
#define PORTA_SERVER_DATA2 6000

void logging();

void main(){
    while (1){
    sleep(1);
    logging();
    }
}

void logging()
{
	unsigned short port;       
    char sendbuf[1024];              
    char recvbuf[1024];              
    struct hostent *hostnm;    
    struct sockaddr_in server; 
    int s;    
    char ch;                 

    FILE *fp;

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(IP_SERVER_DATA);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORTA_SERVER_DATA1);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }

    strcpy(sendbuf, "read");
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf)+1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

	if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
  	{
    	perror("Recv()");
    	exit(6);
  	}
	
	printf("Dado lido1: %s \n", recvbuf);

    /* Fecha o socket */
    close(s);

    printf("%s\n",recvbuf);
    fp = fopen("log.txt", "a");
    fputs("log1//", fp);
    fputs(recvbuf,fp);
    fputs("\n",fp);
    fclose(fp);

        /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(IP_SERVER_DATA);
    if (hostnm == (struct hostent *) 0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORTA_SERVER_DATA2);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect()");
        exit(4);
    }

    strcpy(sendbuf, "read");
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf)+1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

	if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
  	{
    	perror("Recv()");
    	exit(6);
  	}
	
	printf("Dado lido2: %s \n", recvbuf);

    /* Fecha o socket */
    close(s);

    printf("%s\n",recvbuf);
    fp = fopen("log.txt", "a");
    fputs("log2//", fp);
    fputs(recvbuf,fp);
    fputs("\n",fp);
    fclose(fp);
}