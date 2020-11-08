#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdbool.h>

#define PORTA_DIRETORIO 7000
#define ENDERECO_SERVIDOR "localhost"

pthread_mutex_t mutex;

void *SistemaDiretorio(void *arg);
bool testaConexao(int porta);

int servidores[50];
int qtd_servidores = 0;
int ultimo_server = 0;

int main()
{
	char sendbuf[1024];
	char recvbuf[1024];
	struct sockaddr_in client;
	struct sockaddr_in server;

	int s;	/* Socket para aceitar conexoes       */
	int ns; /* Socket conectado ao cliente        */
	int namelen;

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

	if (pthread_mutex_init(&mutex, NULL) != 0)
	{
		perror("A inicializacao do mutex falhou! \n");
		exit(EXIT_FAILURE);
	}

	/*Loop "infinito"*/
	while (1)
	{
		pthread_t thread;

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

		/*Inicializa a thread*/
		if (pthread_create(&thread, NULL, SistemaDiretorio, &ns) != 0)
		{
			printf("Erro na Thread\n");
			exit(1);
		}

		pthread_detach(thread);
	}

	pthread_mutex_destroy(&mutex);
	exit(0);
}

void *SistemaDiretorio(void *arg)
{
	char recvbuf[1024];
	char sendbuf[1024];
	int sockEntrada = *(int *)arg;
	int tentativas = 0;

	if (recv(sockEntrada, recvbuf, sizeof(recvbuf), 0) == -1)
	{
		perror("Recv()");
		exit(6);
	}

	printf("%s conectado \n", recvbuf);

	if (strcmp(recvbuf, "servidor") == 0)
	{
		if (recv(sockEntrada, recvbuf, sizeof(recvbuf), 0) == -1)
		{
			perror("Recv()");
			exit(6);
		}

		if (strcmp(recvbuf, "servidor") == 0)
		{
			printf("Erro ao registrar servidor! \n");
		}
		else
		{
			/*servidores[qtd_servidores] = atoi(recvbuf);
			qtd_servidores++;*/

			for (int i = 0; i <= qtd_servidores; i++)
			{
				if (i == qtd_servidores)
				{
					pthread_mutex_lock(&mutex);
					servidores[qtd_servidores] = atoi(recvbuf);
					qtd_servidores++;
					pthread_mutex_unlock(&mutex);

					printf("Registrei um servidor com a porta %s\n", recvbuf);
					break;
				}
				else if (servidores[i] == 0)
				{
					pthread_mutex_lock(&mutex);
					servidores[i] = atoi(recvbuf);
					pthread_mutex_unlock(&mutex);

					printf("Registrei um servidor com a porta %s\n", recvbuf);
					break;
				}
			}
		}
	}
	else if (strcmp(recvbuf, "cliente") == 0)
	{
		while (tentativas < qtd_servidores)
		{
			if (testaConexao(servidores[ultimo_server]) && servidores[ultimo_server] != 0)
			{
				sprintf(sendbuf, "%d", servidores[ultimo_server]);
				if (send(sockEntrada, sendbuf, strlen(sendbuf) + 1, 0) < 0)
				{
					perror("Send()");
					exit(5);
				}

				ultimo_server = (ultimo_server + 1) % qtd_servidores;
				tentativas = 0;
				
				break;
			}
			else
			{
				ultimo_server = (ultimo_server + 1) % qtd_servidores;
				tentativas++;
			}
		}
	}
}

bool testaConexao(int porta)
{
	char sendbuf[1024];
	char recvbuf[1024];
	struct hostent *hostnm;
	struct sockaddr_in server;
	int s;

	/*
 	* Obtendo o endereco IP do servidor
  	*/
	hostnm = gethostbyname(ENDERECO_SERVIDOR);
	if (hostnm == (struct hostent *)0)
	{
		fprintf(stderr, "Gethostbyname failed\n");
		exit(2);
	}

	/*
  * Define o endereco IP e a porta do servidor
  */
	server.sin_family = AF_INET;
	server.sin_port = htons(porta);
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

	strcpy(sendbuf, "verify");

	if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
	{
		perror("Send()");
		exit(5);
	}

	if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)
	{
		perror("Send()");
		exit(5);
	}

	if (strcmp(recvbuf, "ok") == 0)
	{
		printf("Servidor com a porta %d funcionando. \n", porta);
		return true;
	}
	else
	{
		printf("Servidor com a porta %d fora do ar. \n", porta);
		return false;
	}
}