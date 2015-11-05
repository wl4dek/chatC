#include "Base.h"

typedef struct SocketListaCliente{
    int MySocket;
    int SockToConect;
    Cliente ListaCliente[USERS];
} SocketListaCliente;

pthread_mutex_t     LOCK;

int RegistrarNoChat(Cliente *ListaCliente, char *mensagem){    
    char *IP = strtok(mensagem, "@");
    LimparBuffer();
    char *PORT = strtok(NULL, "@");
    LimparBuffer();
    char *NOME = strtok(NULL, "@");

    printf("IP:%s\n PORT:%s\n NOME:%s", IP, PORT, NOME);

    int i;
    for ( i = 0; i < USERS; ++i) {
        if (ListaCliente[i].ip[0] == '\0') {
            strcpy(ListaCliente[i].ip, IP);
            strcpy(ListaCliente[i].porta, PORT);
            strcpy(ListaCliente[i].nome, NOME);
            return 1;
        }
    }
    return 0;
}

char ExisteNoChat(Cliente *ListaCliente ,char *nome){
    int i;
    for (i = 0; i < USERS; ++i) {
        if ( strcmp(ListaCliente[i].nome, nome) == 0  )
            return '0';
    }
    return '1';
}

char DesconectarDoChat(SocketListaCliente *R, char *mensagem){
    int i;
    int SockToConect;
    for ( i = 0; i < USERS; ++i){
        if (strcmp(mensagem, R->ListaCliente[i].ip) == 0)  {
            
            char enviar[TAM_MENSAGEM];
            strcpy(enviar, "RESPD|1|");
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, R->ListaCliente[i].ip, R->ListaCliente[i].porta);
            EnviarMensagem(enviar, SockToConect);
            close(SockToConect);

            memset((void *) R->ListaCliente[i].ip,0,TAM_IP);
            memset((void *) R->ListaCliente[i].porta,0,TAM_PORT);
            memset((void *) R->ListaCliente[i].nome,0,TAM_NOME);
            return '1';
        }
    }
    return '0';
}

int GetListaCliente(Cliente* ListaCliente, char * mensagem){
    int i;
    memset((void *) mensagem,0,TAM_MENSAGEM);
    strcpy(mensagem, "RESPRL|1|");
    for ( i = 0; i < USERS; ++i){
        if (ListaCliente[i].ip[0] != '\0') {
            strcat(mensagem, ListaCliente[i].ip);
            strcat(mensagem, "@");
            strcat(mensagem, ListaCliente[i].porta);
            strcat(mensagem, "@");
            strcat(mensagem, ListaCliente[i].nome);
            strcat(mensagem, "|");
        }
    }
    return 0;
}

int BroadCastListaCliente(Cliente* ListaCliente, char * mensagem){
    int i;
    memset((void *) mensagem,0,TAM_MENSAGEM);
    strcpy(mensagem, "RESPAL|");
    for ( i = 0; i < USERS; ++i){
        if (ListaCliente[i].ip[0] != '\0') {
            strcat(mensagem, ListaCliente[i].ip);
            strcat(mensagem, "@");
            strcat(mensagem, ListaCliente[i].porta);
            strcat(mensagem, "@");
            strcat(mensagem, ListaCliente[i].nome);
            strcat(mensagem, "|");
        }
    }
    return 0;
}

int ResponderConectado(Cliente* ListaCliente, char * mensagem){
    int i;
    memset((void *) mensagem,0,TAM_MENSAGEM);
    strcpy(mensagem, "RESPR|1|");
    for ( i = 0; i < USERS; ++i){
        if (ListaCliente[i].ip[0] != '\0') {
            strcat(mensagem, ListaCliente[i].ip);
            strcat(mensagem, "@");
            strcat(mensagem, ListaCliente[i].porta);
            strcat(mensagem, "@");
            strcat(mensagem, ListaCliente[i].nome);
            strcat(mensagem, "|");
        }
    }
    return 0;
}

void EnviarListarAll(SocketListaCliente *R, char *mensagem){
    int i;
    int SockToConect;

    BroadCastListaCliente(R->ListaCliente, mensagem);

    for (i = 0; i < USERS; ++i) {
        if (R->ListaCliente[i].ip[0] != '\0'){
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, R->ListaCliente[i].ip, R->ListaCliente[i].porta);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect);
        }
    }
}

void *responder_requisicao(void *arg){
        pthread_mutex_lock(&LOCK);
            SocketListaCliente *R = (SocketListaCliente*)arg;
        pthread_mutex_unlock(&LOCK);
        
        char aux[TAM_MENSAGEM], mensagem[TAM_MENSAGEM];
        char resp;
        int SockToConect;

        if (ReceberMensagem(mensagem, R->MySocket) < 0)  {
            printf("\nErro no recebimento da mensagem\n");
            exit(1);
        }

        if (mensagem[0] != '\0'){
        strcpy(aux, mensagem);
        char *tipo = strtok(aux, "|");

        if (strcmp(tipo, "R") == 0)  {
            char *IpPortaNome = strtok(NULL, "|");
            strcpy(mensagem, IpPortaNome);
            char *Ip = strtok(IpPortaNome, "@");
            char *PORTA = strtok(NULL, "@");
            char *NOME = strtok(NULL, "@");

            resp = ExisteNoChat(R->ListaCliente, NOME);
            if (resp == '1'){
                if (!(RegistrarNoChat(R->ListaCliente, mensagem)))  {
                    memset((void *) mensagem,0,TAM_MENSAGEM);
                    strcpy(mensagem, "RESPR|0|");

                    SockToConect = CriarSocketServidor();
                    ConnectToServer(SockToConect, Ip, PORTA);
                    EnviarMensagem(mensagem, SockToConect);
                    close(SockToConect);
                }else{
                    EnviarListarAll(R, mensagem);
                }
            }
            
            memset((void *) mensagem,0,TAM_MENSAGEM);
            ResponderConectado(R->ListaCliente, mensagem);
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, Ip, PORTA);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect);
        }

        if (strcmp(tipo, "D") == 0) {
            resp = DesconectarDoChat(R, strtok(NULL, "|"));
            if (resp == '1')    {
                EnviarListarAll(R, mensagem);
            }
        }

        if (strcmp(tipo, "RL") == 0) {
            char *Ip = strtok(NULL, "|");
            printf("IP: %s\n", Ip);
            
            int i;
            int index;
            for ( i = 0; i < USERS; ++i )   {
                if (strcmp(R->ListaCliente[i].ip, Ip) == 0)
                {
                    index = i;
                }
            }

            if (GetListaCliente(R->ListaCliente, mensagem) != 0) {
                memset((void *) mensagem,0,TAM_MENSAGEM);
                strcpy(mensagem, "RESPRL|0|");
            }

            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, R->ListaCliente[index].ip, R->ListaCliente[index].porta);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect);
        }
        ImprimirListaContatos(R->ListaCliente);
        resp = ' ';
    }
}