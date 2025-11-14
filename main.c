#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "game.h"

int main(void)
{
   // define server data
   ServerData serverdata;
   if (serverdata_init(&serverdata))
   {
      return 1;
   }

   pthread_t game_thread, server_thread;
   pthread_create(&server_thread, NULL, server_loop, &serverdata);
   pthread_create(&game_thread, NULL, game_loop, &serverdata);

   char buffer[100];
   while (1)
   {
      fgets(buffer, sizeof(buffer), stdin);
      
      // close the server
      if (strcmp(buffer, "close\n") == 0)
      {
         break;
      }
      else
      {
         printf("Not a valid server command\n");
      }
   }
   uint64_t signal = 1;
   write(serverdata.efd, &signal, sizeof(uint64_t));
   pthread_mutex_lock(&serverdata.lock);
   serverdata.signal = signal;
   pthread_mutex_unlock(&serverdata.lock);
   pthread_join(server_thread, NULL);
   pthread_join(game_thread, NULL);
}
