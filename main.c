#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manager.h"
#include "game.h"
#include "server.h"
#include "room.h"

int main(void)
{
   ThreadArgs threadargs;
   if (serverdata_init(&threadargs.serverdata))
   {
      return 1;
   }

   activeplayers_init(&threadargs.activeplayers);

   map_init(&threadargs.startroom);

   pthread_t game_thread, server_thread;
   pthread_create(&server_thread, NULL, server_loop, &threadargs);
   pthread_create(&game_thread, NULL, game_loop, &threadargs);

   char buffer[100];
   while (1)
   {
      fgets(buffer, sizeof(buffer), stdin);
      
      // close the server
      if (strcmp(buffer, "quit\n") == 0)
      {
         break;
      }
      else
      {
         printf("Not a valid server command\n");
      }
   }

   printf("Shutting down...\n");
   uint64_t signal = 1;
   write(threadargs.serverdata.efd, &signal, sizeof(uint64_t));
   pthread_mutex_lock(&threadargs.serverdata.lock);
   threadargs.serverdata.signal = signal;
   pthread_mutex_unlock(&threadargs.serverdata.lock);
   pthread_join(server_thread, NULL);
   pthread_join(game_thread, NULL);
}
