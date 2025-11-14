#ifndef MANAGER_H
#define MANAGER_H

#include "server.h"
#include "game.h"

void process_command(int fd, char* cmd, fd_set *master, ServerData *serverdata)
{
   char *usrcmd = lower(cmd);
   char *response;

   if (strcmp(usrcmd, "hello") == 0)
   {
      response = "Hello, there!";
      reply(fd, "", 0, response, 0);
   }
   else if (strcmp(usrcmd, "quit") == 0)
   {
      response = "Goodbye!";
      reply(fd, "", 0, response, 0);
      close_socket(fd, master, serverdata);
   }
   else
   {
      response = "Hm, I don't know that one.";
   }
   free(usrcmd);
}

void *server_loop(void *args)
{
   ServerData *serverdata = (ServerData*)args;
   printf("Server starting...\n");

   fd_set master;
   fd_set read_fds;
   int fdmax;
   int listener;

   // clear entries
   FD_ZERO(&master);
   FD_ZERO(&read_fds);

   // add event fd to set
   FD_SET(serverdata->efd, &master);
   fdmax = serverdata->efd;

   // listens for new connections
   listener = get_listener_socket(serverdata);

   // add socket to set
   FD_SET(listener, &master);
   fdmax = listener;
   serverdata->listeneridx = listener;

   while (1)
   {
      uint64_t signal = 0;
      // copy socket list
      read_fds = master;

      if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
      {
         perror("Failed to poll sockets");
         break;
      }

      for (int i=0; i <= fdmax; i++)
      {
         // if socket is still in the read set
         if (FD_ISSET(i, &read_fds))
         {
            if (i == listener)
            {
               // new connection
               handle_new_connection(i, &master, &fdmax, serverdata);
            }
            else if (i == serverdata->efd)
            {
               read(serverdata->efd, &signal, sizeof(signal));
            }
            else
            {
               // client sent data
               ClientPkg *clientpkg = get_client_data(i, &master, serverdata);
               char* command = handle_client_data(i, clientpkg, serverdata);
               if (strlen(command) > 0)
               {
                  process_command(i, command, &master, serverdata);
               }
               free(clientpkg);
               free(command);
            }
         }
      }
      if (signal == 1)
         break;
   }
   printf("Server closing...\n");
   return NULL;
}

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
