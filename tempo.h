// para o ftime em milisegundos
#include <sys/types.h>
#include <sys/timeb.h>

// para o clock_gettime em nanosegundos
#include <time.h>

// para as definicoes de erro
#include <errno.h>

void capturar_tempo_nanosegundos(struct timespec *t)
{
   if (clock_gettime(CLOCK_REALTIME,t))
   {
      switch(errno)
      {
         case EFAULT: printf("\nEFAULT tp points outside the accessible address space.\n");
                      break;
         case EINVAL: printf("\nEINVAL The clk_id specified is not supported on this system.\n");
                      break;
         case EPERM:  printf("\nEPERM  clock_settime() does not have permission to set the clock indicated.\n");
                      break;
         default: printf("\nErro indefinido %d\n",errno);
      }
      exit(1);
   }
   return;
}

void capturar_tempo_milissegundos(struct timeb *t)
{
   if (ftime(t))
   {
      printf("\nErro indefinido %d\n",errno);
      exit(1);
   }
   return;
}

/*int main()
{
   struct timespec ti1,tf1;
   struct timeb    ti2,tf2;

   // nanosegundos

   printf("TEMPO EM NANO SEGUNDOS:\n\n");
   capturar_tempo_nanosegundos(&ti1);

   funcionalidade();

   capturar_tempo_nanosegundos(&tf1);

   printf("Tempo ini  (s): %ld\n",(long)ti1.tv_sec);
   printf("Tempo ini (ns): %ld\n",(long)ti1.tv_nsec);

   printf("Tempo fim  (s): %ld\n",(long)tf1.tv_sec);
   printf("Tempo fim (ns): %ld\n",(long)tf1.tv_nsec);

   printf("Resultado em nano segundos (podemos aproximar para micro ou mili)\n");
   printf("Tempo Med (ns): %lld\n\n",(((long long)tf1.tv_sec*(long long)(1000000000))+((long long)tf1.tv_nsec))-(((long long)ti1.tv_sec*(long long)(1000000000)) +((long long)ti1.tv_nsec)));

   // milisegundos (aproximado)

   printf("TEMPO EM MILI SEGUNDOS:\n\n");
   capturar_tempo_milissegundos(&ti2);

   funcionalidade();

   capturar_tempo_milissegundos(&tf2);

   printf("Tempo ini  (s): %ld\n",(long)ti2.time);
   printf("Tempo ini (ms): %ld (aproximado)\n",(long)ti2.millitm);

   printf("Tempo fim  (s): %ld\n",(long)tf2.time);
   printf("Tempo fim (ms): %ld (aproximado)\n",(long)tf2.millitm);

   printf("Tempo     (ms): %ld\n",(((long)tf2.time*1000)+(long)tf2.millitm) -(((long)ti2.time*1000)+(long)ti2.millitm));

   return(0);
}*/
