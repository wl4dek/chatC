#include "Servidor.h"
#define PORTA_SERVIDOR_TCP 9999

int main()  {
    SocketListaCliente  SLC;
    int                 sock;
    pthread_t           requisicao;
    char                mensagem[TAM_MENSAGEM];

    #ifdef _WIN32
        WORD wPackedValues;
        WSADATA  SocketInfo;
        int      nLastError,    nVersionMinor = 1,  nVersionMajor = 1;
        wPackedValues = (WORD)(((WORD)nVersionMinor) << 8) | (WORD)nVersionMajor;
        nLastError = WSAStartup(wPackedValues, &SocketInfo);
    #endif

    LimparTela();

    LimparListaClientes(SLC.ListaCliente);
    sock = CriarSocketServidor();

    CriarBindToServidor(sock, PORTA_SERVIDOR_TCP);
    
    for (;;) {
        
        SLC.MySocket = AceitarConexaoCliente(sock);

        if (pthread_create(&requisicao,NULL,responder_requisicao,(void*)(&SLC)))   {
            printf("\nERRO: Crianção thread.\n");
            return(0);
        }
    }
    pthread_kill(requisicao,0);
}
