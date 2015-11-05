#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

#define TAM_MENSAGEM 404     /* mensagem de maior tamanho */
#define MAXPENDING 5
#define USERS 12

#define TAM_IP 16
#define TAM_PORT 5
#define TAM_NOME 10

typedef struct cliente {
    char    ip[TAM_IP];
    char    porta[TAM_PORT];
    char    nome[TAM_NOME];
} Cliente;

int EnviarMensagem(char *mensagem,int sockCliente) {
    char Buffer[TAM_MENSAGEM + 5];
    int TamanhoMensagem = strlen(mensagem);
    sprintf(Buffer, "%05d%s", TamanhoMensagem, mensagem);

    if (send(sockCliente, Buffer, strlen(Buffer), 0) != strlen(Buffer)) {
        printf("\nErro no envio da mensagem\n");fflush(stdout);
        return(-1);
    }

    printf("\nTCP Servidor: Enviei (%s)\n",mensagem);fflush(stdout);
    return(0);
}

int ReceberMensagem(char *mensagem, int sock) {
    memset((void *) mensagem,0,TAM_MENSAGEM);

    char TamanhoStr[5];
    if (recv(sock, TamanhoStr, 5, 0) < 0) {
        printf("\nErro na recepção da mensagem\n");fflush(stdout);
        return(-1);
    }
    int TAM = atoi(TamanhoStr);
    
    if (recv(sock, mensagem, TAM, 0) < 0) {
        printf("\nErro na recepção da mensagem\n");fflush(stdout);
        return(-1);
    }
    printf("\nTCP Servidor: Recebi (%s)\n",mensagem);fflush(stdout);

    return(0);
}

void LimparTela(void){
    #ifdef _WIN32
       system("cls");
    #else
       system("clear");
    #endif
}

void LimparBuffer(){
    setbuf(stdin, NULL);
    fflush(stdout);
}

void LimparListaClientes(Cliente *ListaCliente){
    int i;
    for ( i = 0; i < USERS; ++i)    {
        memset(ListaCliente[i].ip, 0, sizeof(TAM_IP));
        memset(ListaCliente[i].porta, 0, sizeof(TAM_PORT));
        memset(ListaCliente[i].nome, 0, sizeof(TAM_NOME));
    }
}

int ConnectToServer(int sock,char *IP, char *porta)  {
    struct sockaddr_in endereco; /* Endereço Local */
    int PORTA = atoi(porta);

    /* Construção da estrutura de endereço do servidor */
    memset(&endereco, 0, sizeof(endereco));   /* Zerar a estrutura */
    endereco.sin_family      = AF_INET;       /* Família de endereçamento da Internet */
    endereco.sin_addr.s_addr = inet_addr(IP); /* Endereço IP do Servidor */
    endereco.sin_port        = htons(PORTA);  /* Porta do Servidor */

    /* Estabelecimento da conexão com o servidor de echo */
    int ERRO = connect(sock, (struct sockaddr *) &endereco, sizeof(endereco));
    if (ERRO < 0) {
        printf("\nErro no connect(%d)!\n", ERRO);fflush(stdout);
        return(-1);
    }
    return(0);
}

int CriarSocketServidor(){
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)   {
        printf("\nErro na criação do socket!\n");fflush(stdout);
        return(-1);
    }    
    return sock;
}

int CriarBindToServidor(int sock, int porta) {
    struct sockaddr_in endereco;

    memset(&endereco, 0, sizeof(endereco));       /* Zerar a estrutura */
    endereco.sin_family      = AF_INET;           /* Família de endereçamento da Internet */
    endereco.sin_addr.s_addr = htonl(INADDR_ANY); /* Qualquer interface de entrada */
    endereco.sin_port        = htons(porta);      /* Porta local */

    if (bind(sock, (struct sockaddr *) &endereco, sizeof(endereco)) < 0)    {
       printf("\nErro no bind()!\n");fflush(stdout);
       return(-1);
    }

    if (listen(sock, MAXPENDING) < 0)   {
       printf("\nErro no listen()!\n");fflush(stdout);
       return(-1);
    }    
    return(sock);
}

int AceitarConexaoCliente(int sockServidor) {
    int                socket_cliente;
    struct sockaddr_in endereco;
    int                tamanho_endereco;

    tamanho_endereco = sizeof(endereco);

    if ((socket_cliente = accept(sockServidor, (struct sockaddr *) &endereco, &tamanho_endereco)) < 0) {
        printf("\nErro no accept()!\n");fflush(stdout);
        return(0);
    }
    return(socket_cliente);
}

void ImprimirListaContatos(Cliente *ListaCliente){
    int i;
    printf("ListaCliente :\n");
    for (i = 0; i < USERS; ++i) {
        if (ListaCliente[i].ip[0] != '\0')  {
            printf("Nome[%d]: %s\n", i, ListaCliente[i].nome);   
        }
    }
    printf("--------------------------------------------------\n");
}