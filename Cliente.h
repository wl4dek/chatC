#include "Base.h"
#include "tempo.h"

typedef struct SocketListaCliente{
    int MySocket;
    char        NOME[TAM_NOME]; 
    char        IP[TAM_IP];
    Cliente     ListaCliente[USERS];
    char        UltimarMensagem[5][TAM_MENSAGEM];
} SocketListaCliente;

#define PORTA_SERVIDOR_CLIENTE_TCP 4545

pthread_mutex_t     LOCK;
pthread_mutex_t     TelaPrincipalLOCK;

void PrintLastMenssage(SocketListaCliente *M){
    int i;
    printf("\n--------------------------------------------------\n");
    printf("ULTIMAS MENSAGENS:\n");
    for ( i = 4; i >= 0; --i )
    {
        if (M->UltimarMensagem[i][0] != '\0')
        {
            printf(" %s\n", M->UltimarMensagem[i]);
        }
    }
    printf("--------------------------------------------------\n\n");
}

void AddLastMessage(SocketListaCliente *M, char *mensagem){
    int i;
    for ( i = 4; i >= 0; --i )
    {
        strcpy(M->UltimarMensagem[i+1], M->UltimarMensagem[i]);
    }
    strcpy(M->UltimarMensagem[0], mensagem);
}

void ClearLastMessage(SocketListaCliente *M){
    int i;
    for (i = 0; i < 5; ++i)
    {
        memset(M->UltimarMensagem[i], 0, sizeof(TAM_MENSAGEM));
    }
}

int RegistrarNoChat(Cliente *ListaCliente, char *mensagem){
    /*printf("REGISTRAR O CLIENTE: %s\n", mensagem);*/
    char *IP = strtok(mensagem, "@");
    char *PORT = strtok(NULL, "@");
    char *NOME = strtok(NULL, "@");

    int i;
    for ( i = 0; i < USERS; ++i) {
        if (ListaCliente[i].ip[0] == '\0') {
            strcpy(ListaCliente[i].ip, IP);
            strcpy(ListaCliente[i].porta, PORT);
            strcpy(ListaCliente[i].nome, NOME);

            /*printf("IP: %s\n", ListaCliente[i].ip);
            printf("PORTA: %s\n", ListaCliente[i].porta);
            printf("NOME: %s\n", ListaCliente[i].nome);*/
            return 1;
        }
    }
    return 0;
}

void *ServidorCliente(void *arg){
    pthread_mutex_lock(&LOCK);
        SocketListaCliente *R = (SocketListaCliente*)arg;
    pthread_mutex_unlock(&LOCK);

    int SockToConect;
    char mensagem[TAM_MENSAGEM], aux[TAM_MENSAGEM];

    int sock = CriarSocketServidor();
    CriarBindToServidor(sock, PORTA_SERVIDOR_CLIENTE_TCP);

    for (;;)    {
        R->MySocket = AceitarConexaoCliente(sock);
        ReceberMensagem(mensagem, R->MySocket);

        if (mensagem[0] != '\0')
        {
            strcpy(aux, mensagem);
            char *tipo = strtok(aux, "|");

            if (strcmp(tipo, "RESPAL") == 0) {
                int i = 0;
                LimparListaClientes(R->ListaCliente);

                char *cliente = strtok (NULL,"|");
                char **clientes = (char**)malloc(sizeof(char*)*USERS);

                while (cliente != NULL) {
                    clientes[i] = (char*)malloc(sizeof(char)*(TAM_IP + TAM_NOME + 5));
                    strcpy(clientes[i], cliente);
                    i++;
                    cliente = strtok (NULL, "|");
                }

                for ( i = 0; i < USERS; ++i)
                {
                    if (clientes[i] != NULL)
                    {
                        RegistrarNoChat(R->ListaCliente, clientes[i]);
                    }
                }
            }

            if (strcmp(tipo, "RESPR") == 0)
            {
                if ( strcmp(strtok(NULL, "|"), "1") == 0 )
                {
                    printf("VOCÊ FOI CADASTRADO COM SUCESSO!\n");
                }
            }

            if ( strcmp(tipo, "RESPD") == 0)
            {
                if ( strcmp(strtok(NULL, "|"), "1") == 0 )
                {
                    LimparListaClientes(R->ListaCliente);
                    printf("VOCÊ FOI DESCONECTADO COM SUCESSO!\n");
                }else{
                    printf("ERRO AO DESCONECTAR!\n");
                }
            }

            if ( strcmp(tipo, "B") == 0)
            {
                char *nome = strtok(NULL, "|");
                char *msg = strtok(NULL, "|");

                printf("MENSAGEM BROADCAST:\n");
                printf("%s : %s\n", nome, msg);

                memset((void *) mensagem,0,TAM_MENSAGEM);
                sprintf(mensagem, "BROADCAST %s: %s", nome, msg);
                AddLastMessage(R, mensagem);

                int i;
                for ( i = 0; i < USERS; ++i){
                    if (strcmp(R->ListaCliente[i].nome, nome) == 0) {
                        memset((void *) mensagem,0,TAM_MENSAGEM);
                        sprintf(mensagem, "RESPB|1|%s|",nome );
                        SockToConect = CriarSocketServidor();
                        ConnectToServer(SockToConect, R->ListaCliente[i].ip, R->ListaCliente[i].porta);
                        EnviarMensagem(mensagem, SockToConect);
                        close(SockToConect);
                    }
                }
            }

            if ( strcmp(tipo, "MD") == 0 )
            {
                char *nome = strtok(NULL, "|");
                char *msg = strtok(NULL, "|");

                memset((void *) mensagem,0,TAM_MENSAGEM);
                sprintf(mensagem, "%s: %s", nome, msg);
                printf("%s\n", mensagem);
                AddLastMessage(R, mensagem);

                int i;
                for ( i = 0; i < USERS; ++i){
                    if (strcmp(R->ListaCliente[i].nome, nome) == 0) {
                        memset((void *) mensagem,0,TAM_MENSAGEM);
                        sprintf(mensagem, "RESPMD|1|%s|",nome );
                        SockToConect = CriarSocketServidor();
                        ConnectToServer(SockToConect, R->ListaCliente[i].ip, R->ListaCliente[i].porta);
                        EnviarMensagem(mensagem, SockToConect);
                        close(SockToConect);
                    }
                }
            }
        }
    }
}

void *TelaPrincipal(void *arg){
    for (;;) {
        pthread_mutex_lock(&TelaPrincipalLOCK);
        LimparTela();

        printf("################# MENU ###########################\n");
        printf("#           1  -  Conectar                       #\n");
        printf("#           2  -  Desconectar                    #\n");
        printf("#           3  -  Atualizar Lista Clientes       #\n");
        printf("#           4  -  Mensagem Broadcast             #\n");
        printf("#           5  -  Mensagem Individual            #\n");
        printf("#           6  -  Sair                           #\n");
        printf("##################################################\n\n");

        pthread_mutex_lock(&LOCK);
            SocketListaCliente *R = (SocketListaCliente*)arg;
        pthread_mutex_unlock(&LOCK);

        printf("--------------------------------------------------\n");
        ImprimirListaContatos(R->ListaCliente);

        PrintLastMenssage(R);
    }
}

int ConectarChatServidor(SocketListaCliente *R, char *mensagem){
    memset((void *) mensagem,0,TAM_MENSAGEM);
    char PORTA[6];
    
    strcpy(mensagem, "R|");
    LimparBuffer();
    printf("MEU IP: ");
    scanf("%s", R->IP);
    strcat(mensagem, R->IP);
    sprintf(PORTA, "@%d@", PORTA_SERVIDOR_CLIENTE_TCP);
    strcat(mensagem, PORTA);
    
    LimparBuffer();

    printf("MEU NOME: ");
    scanf("%s", R->NOME);
    strcat(mensagem, R->NOME);
    strcat(mensagem, "|");
    return 0;
}

void Desconectar(char *mensagem, char *Ip){
    sprintf(mensagem, "D|%s|", Ip);
}

void RequisicaoListaCliente (char *mensagem, char *Ip){
    sprintf(mensagem, "RL|%s|", Ip);   
}

void MensagemBroadcast (char *mensagem, SocketListaCliente *R){
    int i;
    int SockToConect;
    char msg[TAM_MENSAGEM];
    printf("QUAL A MENSAGEM QUE VOCÊ GOSTARIA DE ENVIAR PARA TODOS?\n");
    scanf(" %[^\n]s",msg);

    for (i = 0; i < USERS; ++i)
    {
        if(R->ListaCliente[i].ip[0] != '\0')
        {
            memset((void *) mensagem,0,TAM_MENSAGEM);
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, R->ListaCliente[i].ip, R->ListaCliente[i].porta);
            sprintf(mensagem, "B|%s|%s|",R->NOME,  msg);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect); 
        }
    }
}

void MensagemIndividual (SocketListaCliente *R){
    int indice;
    int SockToConect;
    char mensagem[TAM_MENSAGEM];
    char texto[TAM_MENSAGEM];

    printf("DIGITE O INDICE DO CLIENTE QUE DESEJA MANDAR A MENSAGEM:\n");
    ImprimirListaContatos(R->ListaCliente);
    scanf("%d", &indice);

    printf("DIGITE A MENSAGEM:\n");
    scanf(" %[^\n]s", texto);

    SockToConect = CriarSocketServidor();
    ConnectToServer(SockToConect, R->ListaCliente[indice].ip, R->ListaCliente[indice].porta);
    sprintf(mensagem, "MD|%s|%s|", R->NOME, texto);
    EnviarMensagem(mensagem, SockToConect);
    close(SockToConect);
}