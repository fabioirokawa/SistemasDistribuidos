#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NOME_SEMAFORO "/semaforo1"
#define PORTA_SERVER_DATA 5000
#define SHM_KEY 5000

void modificaDado(char *dado, sem_t *semaphore, char *share_memory);
void guardaLog(char *dado, FILE *fp1);
char *recuperaLog(char *share_memory);

int main(int argc, char **argv)
{
    unsigned short port;
    char sendbuf[1024];
    char recvbuf[1024];
    struct sockaddr_in client;
    struct sockaddr_in server;
    int s;  /* Socket para aceitar conexoes       */
    int ns; /* Socket conectado ao cliente        */
    int shmid;
    int namelen;
    pid_t pid, fid;
    FILE *fp;

    //Criando memoria compartilhada
    shmid = shmget(SHM_KEY, 1024, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        printf("shmget error\n");
    }

    //Associando memoria compartilhda a um ponteiro
    char *share_memory = (char *)shmat(shmid, NULL, 0);

    //Inicializando a memoria compartilhada com um valor qualquer
    strcpy(share_memory, "NADA");

    //Desativando algum semaforo com o mesmo nome por garantia
    sem_unlink(NOME_SEMAFORO);

    //Criando semaforo
    sem_t *semaphore = sem_open(NOME_SEMAFORO, O_CREAT, 0644, 1);
    if (semaphore == SEM_FAILED)
    {
        perror("Erro ao inicializar semaforo");
        exit(EXIT_FAILURE);
    }

    //Recuperando dados anteriores
    recuperaLog(share_memory);

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
    server.sin_port = htons(PORTA_SERVER_DATA);
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

            if (strcmp(recvbuf, "write") == 0)
            {
                if (recv(ns, recvbuf, sizeof(recvbuf), 0) == -1)
                {
                    perror("Recv()");
                    exit(6);
                }
                guardaLog(recvbuf, fp);
                modificaDado(recvbuf, semaphore, share_memory);
                printf("Dado recebido: %s\n", recvbuf);
            }
            else if (strcmp(recvbuf, "read") == 0)
            {
                strcpy(sendbuf, share_memory);
                /* Envia uma mensagem ao cliente atraves do socket conectado */
                if (send(ns, sendbuf, strlen(sendbuf) + 1, 0) < 0)
                {
                    perror("Send()");
                    exit(7);
                }
                printf("Dado enviado: %s \n", sendbuf);
            }

            /* Fecha o socket conectado ao cliente */
            close(ns);
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

    /* Detach the shared memory segment.  */
    shmdt(share_memory);

    /* Deallocate the shared memory segment.  */
    shmctl(shmid, IPC_RMID, 0);

    /*Fechando o semaforo*/
    sem_close(semaphore);
}

void modificaDado(char *dado, sem_t *semaphore, char *share_memory)
{
    //Travando o semaforo
    sem_wait(semaphore);

    //Copiando os dados requisitados para a memoria compartilhada
    strcpy(share_memory, dado);

    //Liberando o semaforo
    sem_post(semaphore);
}

void guardaLog(char *dado, FILE *fp1)
{
    fp1 = fopen("log_s1.txt", "a");
    fputs("log1 \n", fp1);
    fputs(dado, fp1);
    fputs("\n", fp1);
    fclose(fp1);
}

char *recuperaLog(char *share_memory)
{
    FILE *fp;
    fp = fopen("log_s1.txt", "r");
    
    if (fp != NULL)
    {

        char linha[1024];
        char linha_aux[1024];

        do
        {
            fflush(stdout);

            strcpy(linha, linha_aux);
            fgets(linha_aux, 1024, fp);

        } while (strcmp(linha_aux, linha) != 0);

        printf("Recuperacao de log: %s \n", linha);
        strcpy(share_memory, linha);
        fclose(fp);
        return linha;
    }

    return "vazio";
}