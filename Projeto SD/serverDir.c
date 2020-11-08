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
#define PORTA_SERVER_DATA 5000
#define PORTA_DIRETORIO 7500
#define MAX_PORTS 10

int guardaPorta(int port, int *ports);
int pegaPorta(int *ports);

/*
 * Servidor TCP
 */
int main(int argc, char **argv)
{
	unsigned short port;
	char sendbuf[1024];
	char recvbuf[1024];
	char dado[1024];
	struct sockaddr_in client;
	struct sockaddr_in server;
	int s;	/*Socket para aceitar conexoes*/
	int ns; /*Socket conectado ao cliente */
	int namelen;
	int length;
	pid_t pid, fid;

    int portas[MAX_PORTS];
    int porta = -1;

    for (int i = 0; i<MAX_PORTS; i++)
    {
        portas[i] = -1;
    }

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
	server.sin_port = htons(PORTA_DIRETORIO);
	server.sin_addr.s_addr = INADDR_ANY;

	/*
     * Liga o servidor a porta definida anteriormente.
     */

	if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Bind()");
		exit(3);
	}

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


				/* Recebe uma mensagem do cliente atraves do novo socket conectado */
				if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
				{
					perror("Recv()");
					exit(6);
				}

				if (strcmp(recvbuf, "servidor") == 0)
				{
					if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
					{
						perror("Recv()");
						exit(6);
					}
                    port = atoi(recvbuf);
					guardaPorta(porta, portas);
				}
				else if (strcmp(recvbuf, "cliente") == 0)
				{
					port = pegaPorta(portas);
                    sprintf(sendbuf,"%d", porta);
					/* Envia uma mensagem ao cliente atraves do socket conectado */
					if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
					{
						perror("Send()");
						exit(7);
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

int guardaPorta(int port, int *ports){
    for (int i=0; i<MAX_PORTS; i++){
        if (ports[i] == -1)
        {
            ports[i] = port;
            return 0;
        }        
        printf("PORT [%d] is %d\n",i,ports[i]);
    }
    printf("Sem espaço para portas!\n");
    return 0;
}

int pegaPorta(int *ports){
    char tmp[MAX_PORTS];
	char recvbuf[1024];
    for (int i=0; i<MAX_PORTS; i++){
        if (ports[i] != -1) //tem porta e vou ver se tem mto cliente
        {
            //codigo pra se conectar ao servidor com porta
            if (recvbuf == "YESH"){ //to de boa manda ae
                return ports[i];
            }
        }        
    }
    return -1;
}