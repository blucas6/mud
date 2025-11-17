#ifndef MANAGER_H
#define MANAGER_H

#include "server.h"
#include "game.h"

typedef struct {
   ServerData serverdata;
   ActivePlayers activeplayers;
   struct Room startroom;
} ThreadArgs;

void process_command(int fd, char* cmd, fd_set *master, ServerData *serverdata, PlayerData *pd)
{
   char *usrcmd = lower(cmd);
   char *response;

   if (strcmp(usrcmd, "hello") == 0)
   {
      response = "Hello, there!";
      reply(fd, "", 0, response, 0, pd);
   }
   else if (strcmp(usrcmd, "quit") == 0)
   {
      response = "Goodbye!";
      reply(fd, "", 0, response, 0, pd);
      close_socket(fd, master, serverdata);
   }
   else if (strcmp(usrcmd, "look") == 0)
   {
      response = get_nearby_rooms(pd->croom);
      reply(fd, "", 0, response, 0, pd);
      free(response);
   }
   else if (strcmp(usrcmd, "greeting") == 0)
   {
      response = "Hello, you are connected\n\r";
      reply(fd, "", 0, response, 0, pd);
   }
   else
   {
      response = "Hm, I don't know that one.";
      reply(fd, "", 0, response, 0, pd);
   }
   free(usrcmd);
}

void arrived_inroom(int fd, ClientData *client, PlayerData *pd, struct Room *room)
{
   pd->croom = room;
   char *intro = pd->croom->intro;
   printf("Room: %s\n", intro);
   reply(fd, client->inputbuf, client->inputcount, intro, 1, pd);
}

void *server_loop(void *args)
{
   ThreadArgs *threadargs = (ThreadArgs*)args;
   ServerData *serverdata = &threadargs->serverdata;
   ActivePlayers *activeplayers = &threadargs->activeplayers;
   struct Room *startroom = &threadargs->startroom;

   printf("Server starting...\n");

   fd_set master;
   fd_set read_fds;
   int fdmax;
   int listener;

   // clear entries
   FD_ZERO(&master);
   FD_ZERO(&read_fds);

   // add event fd to set
   pthread_mutex_lock(&serverdata->lock);
   FD_SET(serverdata->efd, &master);
   fdmax = serverdata->efd;

   // listens for new connections
   listener = get_listener_socket();
   serverdata->clientlist[0] = listener;
   serverdata->listeneridx = 0;
   pthread_mutex_lock(&activeplayers->lock);
   pthread_mutex_unlock(&activeplayers->lock);

   // add socket to set
   FD_SET(listener, &master);
   fdmax = listener;
   pthread_mutex_unlock(&serverdata->lock);

   printf("Done setting up listener socket\n");

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

      for (int fd=0; fd <= fdmax; fd++)
      {
         // if socket is still in the read set
         if (FD_ISSET(fd, &read_fds))
         {
            // listener socket
            if (fd == listener)
            {
               // new connection
               int newfd = handle_new_connection(fd, &master, &fdmax, serverdata);
               if (newfd != -1)
               {
                  printf("Creating new player\n");
                  // find index of socket
                  pthread_mutex_lock(&serverdata->lock);
                  int index = -1;
                  for (int ix=0; ix<MAX_CLIENTS; ix++)
                  {
                     if (newfd == serverdata->clientlist[ix])
                        index = ix;
                  }
                  if (index == -1) continue;
                  pthread_mutex_lock(&activeplayers->lock);
                  PlayerData *pd = &activeplayers->players[index];
                  new_player_init(pd);
                  process_command(newfd, "greeting", &master, serverdata, pd);
                  arrived_inroom(newfd, &serverdata->clientdata[index], pd, startroom);
                  pthread_mutex_unlock(&activeplayers->lock);
                  pthread_mutex_unlock(&serverdata->lock);
               }
               else
               {
                  printf("Failed to create new player\n");
               }
               continue;
            }
            // event socket
            pthread_mutex_lock(&serverdata->lock);
            if (fd == serverdata->efd)
            {
               read(serverdata->efd, &signal, sizeof(signal));
               pthread_mutex_unlock(&serverdata->lock);
               continue;
            }
            // client socket
            ClientPkg *clientpkg = get_client_data(fd, &master, serverdata);
            for (int ix=0; ix<MAX_CLIENTS; ix++)
            {
               if (serverdata->clientlist[ix] == fd)
               {
                  ClientData *client = &serverdata->clientdata[ix];
                  pthread_mutex_lock(&activeplayers->lock);
                  PlayerData *pd = &activeplayers->players[ix];
                  char* command = handle_client_data(fd, clientpkg, client);
                  if (strlen(command) > 0)
                  {
                     process_command(fd, command, &master, serverdata, pd);
                  }
                  free(command);
                  pthread_mutex_unlock(&activeplayers->lock);
               }
            }
            free(clientpkg);
            pthread_mutex_unlock(&serverdata->lock);
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
   ThreadArgs *threadargs = (ThreadArgs*)args;
   ServerData *serverdata = &threadargs->serverdata;
   ActivePlayers *activeplayers = &threadargs->activeplayers;

   printf("Game thread starting...\n");
   while (1)
   {
      // check for close signal
      uint64_t signal = 0;
      pthread_mutex_lock(&serverdata->lock);
      signal = serverdata->signal;
      pthread_mutex_unlock(&serverdata->lock);
      if (signal) break;

      sleep(20);
      printf("Client list: [");
      for (int ix=0; ix<MAX_CLIENTS; ix++)
      {
         pthread_mutex_lock(&serverdata->lock);
         int fd = serverdata->clientlist[ix];
         printf(" %d", fd);
         if (fd != 0 && ix != serverdata->listeneridx)
         {
            char *msg = "[Game message]: pulse";
            ClientData *client = &serverdata->clientdata[ix];
            pthread_mutex_lock(&activeplayers->lock);
            PlayerData *pd = &activeplayers->players[ix];
            reply(fd, client->inputbuf, client->inputcount, msg, 1, pd);
            pthread_mutex_unlock(&activeplayers->lock);
         }
         pthread_mutex_unlock(&serverdata->lock);
      }
      printf("]\n");
   }
   printf("Game thread closing...\n");
   return NULL;
}

#endif
