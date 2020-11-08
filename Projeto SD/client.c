#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Cliente TCP
 */

int main(int argc, char **argv)
{
  unsigned short port;
  char sendbuf[1024];
  char recvbuf[1024];
  struct hostent *hostnm;
  struct sockaddr_in server;
  int s, opcao = -1;

  /*
  * O primeiro argumento (argv[1]) e o hostname do servidor.
  * O segundo argumento (argv[2]) e a porta do servidor.
  */
  if (argc != 3)
  {
    fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
    exit(1);
  }

  /*
  * Obtendo o endereco IP do servidor
  */

  hostnm = gethostbyname(argv[1]);
  if (hostnm == (struct hostent *)0)
  {
    fprintf(stderr, "Gethostbyname failed\n");
    exit(2);
  }

  port = (unsigned short)atoi(argv[2]);

  /*
  * Define o endereco IP e a porta do servidor
  */
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
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

  while (opcao != 0)
  {
    printf("1 - Escrever\n");
    printf("2 - Ler\n");
    printf("3 - Estado dos servidores\n");
    scanf("%d", &opcao);

    switch (opcao)
    {
      case 0:
      {
        strcpy(sendbuf, "exit");
        if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
        {
          perror("Send()");
          exit(5);
        }
        break;
      }

      case 1:
      {
        //escrever dado
        strcpy(sendbuf, "write");
        if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
        {
          perror("Send()");
          exit(5);
        }

        sleep(0.25);

        strcpy(sendbuf, "Dado do cliente para o server");
        if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)
        {
          perror("Send()");
          exit(5);
        }
        system("clear");
        break;
      }

      case 2:
      {
        //ler dado
        strcpy(sendbuf, "read");
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
        
        system("clear");
        printf("Dado armazenado: %s\n\n\n", recvbuf);
        break;
      }

      case 3:
      {
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
        
        system("clear");
        printf("Status do serveidor: %s\n\n\n", recvbuf);
        break;
      }

      default:
      {
        printf("Opcao Invalida, tente novamente.");
        break;
      }
    }
  }

  close(s);

  exit(5);

  strcpy(sendbuf, "Requisicao");

  /* Envia a mensagem no buffer de envio para o servidor */

  if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0)

  {

    perror("Send()");

    exit(5);
  }

  printf("Mensagem enviada ao servidor: %s\n", sendbuf);

  /* Recebe a mensagem do servidor no buffer de recepcao */

  if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0)

  {

    perror("Recv()");

    exit(6);
  }

  printf("Mensagem recebida do servidor: %s\n", recvbuf);

  /* Fecha o socket */

  close(s);

  printf("Cliente terminou com sucesso.\n");

  exit(0);
}
