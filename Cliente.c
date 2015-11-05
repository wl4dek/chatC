#include "Cliente.h"
#define PORTA_SERVIDOR_TCP "9991"

int main(int argc, char *argv[])
{
    SocketListaCliente      SLC;
    char                    mensagem[TAM_MENSAGEM];
    char                    IP_SERVIDOR[TAM_IP];
    int                     FIM = 1;
    int                     opcao;
    int                     SockToConect;

    pthread_t               ClienteServidor;
    pthread_t               TelaPrinciaplTHREAD;

    #ifdef _WIN32
        WORD wPackedValues;
        WSADATA  SocketInfo;
        int      nLastError,
    	         nVersionMinor = 1,
    	         nVersionMajor = 1;
        wPackedValues = (WORD)(((WORD)nVersionMinor)<< 8)|(WORD)nVersionMajor;
        nLastError = WSAStartup(wPackedValues, &SocketInfo);
    #endif

    if (argc != 2)  {
        printf("Uso: %s <IP Servidor>\n", argv[0]);
        return(1);
    }
    memset((void *) IP_SERVIDOR,0,TAM_IP);
    strcpy(IP_SERVIDOR, argv[1]);

    LimparListaClientes(SLC.ListaCliente);
    ClearLastMessage(&SLC);

    if (pthread_create(&TelaPrinciaplTHREAD,NULL,TelaPrincipal,(void*)(&SLC)))   {
        printf("\nERRO: Crianção thread.\n");
        return(0);
    }    

    if (pthread_create(&ClienteServidor,NULL,ServidorCliente,(void*)(&SLC)))   {
        printf("\nERRO: Crianção thread.\n");
        return(0);
    }

    while(FIM){
        scanf("%d", &opcao);
        if (opcao == 1) {
            ConectarChatServidor(&SLC, mensagem);

            struct      timespec    ti,tf;
            
            capturar_tempo_nanosegundos(&ti);
            
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, IP_SERVIDOR, PORTA_SERVIDOR_TCP);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect);
            
            capturar_tempo_nanosegundos(&tf);

            memset((void *) mensagem,0,TAM_MENSAGEM);
            sprintf(mensagem, "Conexao Estabelecia em: %lld nanosegundos\n", (((long long)tf.tv_sec*(long long)(1000000000))+((long long)tf.tv_nsec))-(((long long)ti.tv_sec*(long long)(1000000000)) +((long long)ti.tv_nsec)));

            AddLastMessage(&SLC, mensagem);
        }

        if (opcao == 2)
        {
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, IP_SERVIDOR, PORTA_SERVIDOR_TCP);
            Desconectar(mensagem, SLC.IP);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect);   
        }

        if (opcao == 3)
        {
            SockToConect = CriarSocketServidor();
            ConnectToServer(SockToConect, IP_SERVIDOR, PORTA_SERVIDOR_TCP);
            RequisicaoListaCliente(mensagem, SLC.IP);
            EnviarMensagem(mensagem, SockToConect);
            close(SockToConect); 
        }

        if (opcao == 4)
        {
            LimparTela();
            MensagemBroadcast(mensagem, &SLC);
        }

        if (opcao == 5)
        {
            LimparTela();
            MensagemIndividual(&SLC);
        }

        if (opcao == 6)
        {
            LimparListaClientes(SLC.ListaCliente);
            FIM = 0;
        }

        pthread_mutex_unlock(&TelaPrincipalLOCK);
    }

    return(0);
}
