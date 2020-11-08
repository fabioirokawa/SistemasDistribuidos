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
#define PORTA_SERVER_DATA1 5000
#define PORTA_SERVER_DATA2 6000

#define IP_DIRETORIO "localhost"
#define PORTA_DIRETORIO 7000

void guardaDadoS1(char *dado);
void guardaDadoS2(char *dado);
void leDadoS1(char *dado);
void leDadoS2(char *dado);
void registrarNoDiretorio(int portaServer);

int main(int argc, char **argv)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    char dado[1024];
    char dadoCopia[1024];
    struct sockaddr_in client;
    struct sockaddr_in server;
    int s;  /*Socket para aceitar conexoes*/
    int ns; /*Socket conectado ao cliente */
    int namelen;
    int length;
    int portaServer;
    pid_t pid, fid;

    //primeiro se conecta ao diretorio

    /*
     * Cria um socket TCP (stream) para aguardar conexoes
     */

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket()");
        exit(2);
    }

    /*
    * Define a qual endereco IP e porta o servidor estara ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os enderecos IP
    */

    server.sin_family = AF_INET;
    server.sin_port = 0;
    server.sin_addr.s_addr = INADDR_ANY;

    /*
     * Liga o servidor a porta definida anteriormente.
     */

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind()");
        exit(3);
    }

    /* Imprime o numero da porta */
    length = sizeof(server);
    if (getsockname(s, (struct sockaddr *)&server, &length) < 0)
    {
        perror("getting socket name");
        exit(1);
    }
    portaServer = ntohs(server.sin_port);
    printf("Socket port #%d\n", ntohs(server.sin_port));

    registrarNoDiretorio(portaServer);

    /*
     * Prepara o socket para aguardar por conexoes e
     * cria uma fila de conexoes pendentes.
     */

    if (listen(s, 1) != 0)
    {
        perror("Listen()");
        exit(4);
    }

    while (1)
    {
        /*
	  	* Aceita uma conexao e cria um novo socket atraves do qual
	  	* ocorrera a comunicacao com o cliente.
		*/
        namelen = sizeof(client);
        if ((ns = accept(s, (struct sockaddr *)&client, (socklen_t *)&namelen)) == -1)
        {
            perror("Accept()");
            exit(5);
        }

        if ((pid = fork()) == 0)
        {
            /*
			* Processo filho 
			*/

            /* Fecha o socket aguardando por conexoes */
            close(s);

            /* Processo filho obtem seu proprio pid */
            fid = getpid();

            while (strcmp(recvbuf, "exit") != 0)
            {
                /* Recebe uma mensagem do cliente atraves do novo socket conectado */
                if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
                {
                    perror("Recv()");
                    exit(6);
                }

                if (strcmp(recvbuf, "write") == 0)
                {
                    if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
                    {
                        perror("Recv()");
                        exit(6);
                    }
                    guardaDadoS1(recvbuf);
                    guardaDadoS2(recvbuf);
                }
                else if (strcmp(recvbuf, "read") == 0)
                {
                    leDadoS1(dado);
                    leDadoS2(dadoCopia);

                    if (strcmp(dado, dadoCopia) != 0)
                    {
                        strcpy(sendbuf, "Dado Inconsistente. \n");
                    }
                    else
                    {
                        strcpy(sendbuf, dado);
                    }

                    /* Envia uma mensagem ao cliente atraves do socket conectado */
                    if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(7);
                    }
                }
                else if (strcmp(recvbuf, "verify") == 0)
                {
                    strcpy(sendbuf, "ok");

                    /* Envia uma mensagem ao cliente atraves do socket conectado */
                    if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                    {
                        perror("Send()");
                        exit(7);
                    }
                    printf("Status enviado. \n");
                }
            }

            /* Fecha o socket conectado ao cliente */
            close(ns);

            /* Processo filho termina sua execucao */
            printf("[%d] Processo filho terminado com sucesso.\n", fid);

            exit(0);
        }

        else
        {
            /*
			* Processo pai 
			*/
            if (pid > 0)
            {
                printf("Processo filho criado: %d\n", pid);
                /* Fecha o socket conectado ao cliente */
                close(ns);
            }
            else
            {
                perror("Fork()");
                exit(7);
            }
        }
    }
}

void guardaDadoS1(char *dado)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(IP_SERVER_DATA);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTA_SERVER_DATA1);
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

    strcpy(sendbuf, "write");
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    sleep(1);

    strcpy(sendbuf, dado);
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("Dado enviado ao servidor de dados. \n");

    /* Fecha o socket */
    close(s);
}

void guardaDadoS2(char *dado)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(IP_SERVER_DATA);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTA_SERVER_DATA2);
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

    strcpy(sendbuf, "write");
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    sleep(1);

    strcpy(sendbuf, dado);
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    printf("Dado enviado ao servidor de dados. \n");

    /* Fecha o socket */
    close(s);
}

void leDadoS1(char *dado)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(IP_SERVER_DATA);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTA_SERVER_DATA1);
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
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }

    printf("Dado lido: %s \n", recvbuf);
    strcpy(dado, recvbuf);

    /* Fecha o socket */
    close(s);
}

void leDadoS2(char *dado)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(IP_SERVER_DATA);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTA_SERVER_DATA2);
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
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
    {
        perror("Recv()");
        exit(6);
    }

    printf("Dado lido: %s \n", recvbuf);
    strcpy(dado, recvbuf);

    /* Fecha o socket */
    close(s);
}

void registrarNoDiretorio(int portaServer)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;

    hostnm = gethostbyname(IP_DIRETORIO);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORTA_DIRETORIO);
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

    strcpy(sendbuf, "servidor");
    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }

    sleep(5);

    /* Convertendo int para string */
    sprintf(sendbuf, "%d", portaServer);

    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
    {
        perror("Send()");
        exit(5);
    }
    
    /* Fecha o socket */
    close(s);
}