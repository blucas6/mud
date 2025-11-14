#ifndef GAME_H
#define GAME_H

#include "server.h"

void *game_loop(void *args)
{
   printf("Game thread starting...\n");
   ServerData *serverdata = (ServerData*)args;
   while (1)
   {
      // check for close signal
      uint64_t signal = 0;
      pthread_mutex_lock(&serverdata->lock);
      signal = serverdata->signal;
      pthread_mutex_unlock(&serverdata->lock);
      if (signal) break;

      sleep(10);
      for (int i=0; i<MAX_CLIENTS; i++)
      {
         pthread_mutex_lock(&serverdata->lock);
         int fd = serverdata->clientlist[i];
         if (fd != 0 && i != serverdata->listeneridx)
         {
            char *msg = "pulse";
            ClientData *client = &serverdata->clientdata[i];
            reply(fd, client->inputbuf, client->inputcount, msg, 1);
         }
         pthread_mutex_unlock(&serverdata->lock);
      }
   }
   printf("Game thread closing...\n");
   return NULL;
}

#endif
