#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/eventfd.h>

#include "commands.h"

#define PORT "8888"
#define IP "127.0.0.1"
#define PENDING_CN 10

#define IAC 255
#define WILL 251
#define WONT 252
#define DO 253
#define DONT 254
#define SB 250
#define SE 240
#define TELOPT_NAWS 31
#define ECHO 1
#define SUPRESS 3
#define LINEMODE 34

#define MAX_W 250
#define MAX_H 80

#define MAX_CLIENTS 8

typedef struct {
   int nbytes;
   unsigned char buffer[256];
} ClientPkg;

typedef struct {
   int width;
   int height;
   char inputbuf[256];
   int inputcount;
   pthread_mutex_t lock;
} ClientData;

typedef struct {
   int efd;
   uint64_t signal;
   int listeneridx;
   int clientlist[MAX_CLIENTS];
   ClientData clientdata[MAX_CLIENTS];
   pthread_mutex_t lock;
} ServerData;

void clientdata_init(ClientData *client)
{
   client->width = 0;
   client->height = 0;
   memset(client->inputbuf, 0, sizeof(client->inputbuf));
   client->inputcount = 0;
   pthread_mutex_init(&client->lock, NULL);
}

int serverdata_init(ServerData *serverdata)
{
   serverdata->signal = 0;
   serverdata->efd = eventfd(0,0);
   pthread_mutex_init(&serverdata->lock, NULL);
   if (serverdata->efd == -1)
   {
      perror("Failed to create event fd");
      return 1;
   }
   for (int i=0; i<MAX_CLIENTS; i++)
   {
      serverdata->clientlist[i] = 0;
      clientdata_init(&serverdata->clientdata[i]);
   }
   return 0;
}

void close_socket(int fd, fd_set *master, ServerData *serverdata)
{
   pthread_mutex_lock(&serverdata->clientdata[fd].lock);
   serverdata->clientlist[fd] = 0;
   clientdata_init(&serverdata->clientdata[fd]);
   pthread_mutex_unlock(&serverdata->clientdata[fd].lock);
   close(fd);
   // remove from master list
   FD_CLR(fd, master);
}

/* Adds a connection to the master list */
void handle_new_connection(int listener, fd_set *master, int *fdmax, ServerData *serverdata) 
{
   socklen_t addrlen;
   int newfd;
   struct sockaddr_storage remoteaddr;

   addrlen = sizeof(remoteaddr);
   newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
   if (newfd == -1)
   {
      perror("Failed to accept socket");
   }
   else if (newfd >= MAX_CLIENTS)
   {
      printf("Warning: cannot handle new connection!\n");
      printf("List of clients full! %d -> %d\n", newfd, MAX_CLIENTS);
      close_socket(newfd, master, serverdata);
   }
   else
   {
      // add valid client to master list
      FD_SET(newfd, master);
      if (newfd > *fdmax)
         *fdmax = newfd;

      // lock client before editing server data
      pthread_mutex_lock(&serverdata->clientdata[newfd].lock);
      serverdata->clientlist[newfd] = newfd;
      serverdata->clientdata[newfd].width = 0;
      serverdata->clientdata[newfd].height = 0;
      serverdata->clientdata[newfd].inputcount = 0;
      printf("Client List: [");
      for (int i=0; i<MAX_CLIENTS; i++)
      {
         printf("%d ", serverdata->clientlist[i]);
      }
      printf("]\n");
      pthread_mutex_unlock(&serverdata->clientdata[newfd].lock);

      // print ip 
      struct sockaddr_in *sa = (struct sockaddr_in*)&remoteaddr;
      char buf[20];
      const char *ip = inet_ntop(remoteaddr.ss_family, &(sa->sin_addr), buf, sizeof(buf));
      printf("New connection from %s on socket %d\n", ip, newfd);

      // send telnet NAWS negotiation
      unsigned char cmd[] = { IAC, DO, TELOPT_NAWS, IAC, WILL, ECHO, IAC, DO, SUPRESS, IAC, DO, LINEMODE};
      send(newfd, cmd, sizeof(cmd), 0);

      // greeting
      char *greeting = "Hello, you are connected\n\r";
      send(newfd, greeting, strlen(greeting), 0);
   }
}

int reply(int fd, char *inputbuffer, int inputcount, char *reply, int sendinput)
{
   char msg[256];
   int pos = 0;
   if (sendinput && inputcount > 0)
   {
      msg[pos++] = '\n';
      msg[pos++] = '\r';
   }
   memcpy(msg+pos, reply, strlen(reply));
   pos += strlen(reply);
   msg[pos++] = '\n';
   msg[pos++] = '\r';
   if (sendinput && inputcount > 0)
   {
      memcpy(msg+pos, inputbuffer, inputcount);
      pos += inputcount;
   }
   if (send(fd, msg, pos, 0) == -1)
   {
      perror("Reply failed");
      return 1;
   }
   return 0;
}

/* Creates a socket to listen for connections */
int get_listener_socket(ServerData* serverdata)
{
   struct addrinfo hints, *ai, *p;
   int listener;
   int reuseport = 1;

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   if (getaddrinfo(IP, PORT, &hints, &ai) == -1)
   {
      perror("Failed to get socket info");
      return 1;
   }

   for (p = ai; p != NULL; p = p->ai_next)
   {
      listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (listener < 0) continue;
      // reuse sockets
      setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuseport, sizeof(int));
      if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
      {
         close(listener);
         continue;
      }
      struct sockaddr_in *sa = (struct sockaddr_in*)p->ai_addr;
      int port = ntohs(sa->sin_port);
      printf("Listening on Port: %d\n", port);
      break;
   }
   
   if (p == NULL)
   {
      fprintf(stderr, "Failed to bind!\n");
      return 1;
   }

   freeaddrinfo(ai);

   if (listen(listener, PENDING_CN) == -1)
   {
      perror("Socket listen");
      return 1;
   }

   pthread_mutex_lock(&serverdata->clientdata[listener].lock);
   serverdata->clientlist[listener] = listener;
   pthread_mutex_unlock(&serverdata->clientdata[listener].lock);

   return listener;
}

void set_client_wh(int fd, int width, int height, ClientData *client)
{
   printf("Client %d settings: %dx%d\n", fd, width, height);
   pthread_mutex_lock(&client->lock);
   client->width = width;
   client->height = height;
   pthread_mutex_unlock(&client->lock);
}

char* handle_client_data(int fd, ClientPkg *clientpkg, ServerData *serverdata)
{
   int commandsz = 256;
   char *command = malloc(commandsz);
   command[0] = '\0';

   unsigned char *buf = clientpkg->buffer;
   // raw data
   printf("Raw: [");
   for (int i=0; i<clientpkg->nbytes; i++)
   {
      printf("%d ", buf[i]);
   }
   printf("]\n");

   ClientData *client = &serverdata->clientdata[fd];

   // NAWS accepted
   if (buf[0] == IAC && buf[1] == WILL && buf[2] == TELOPT_NAWS
      && buf[3] == IAC && buf[4] == SB && buf[5] == TELOPT_NAWS)
   {
      int width = (buf[6] << 8) | buf[7];
      int height = (buf[8] << 8) | buf[9];
      set_client_wh(fd, width, height, client);
   }
   else if (buf[0] == IAC && buf[1] == SB && buf[2] == TELOPT_NAWS)
   {
      int width = (buf[3] << 8) | buf[4];
      int height = (buf[5] << 8) | buf[6];
      set_client_wh(fd, width, height, client);
   }
   // Back space character
   else if (buf[0] == 8 || buf[0] == 127)
   {
      pthread_mutex_lock(&client->lock);
      client->inputbuf[client->inputcount--] = ' ';
      send(fd, "\b \b", 3, 0);
      pthread_mutex_unlock(&client->lock);
   }
   // Command sent
   else if(buf[0] == '\r')
   {
      // send back newline
      pthread_mutex_lock(&client->lock);
      send(fd, "\r\n", 2, 0);
      // handle command
      client->inputbuf[client->inputcount++] = '\0';
      printf("Received from %d: %s\n", fd, client->inputbuf);
      strcpy(command, client->inputbuf);
      // clear input buffer
      client->inputbuf[0] = '\0';
      client->inputcount = 0;
      pthread_mutex_unlock(&client->lock);
   }
   // Single character input - save to input buffer
   else
   {
      pthread_mutex_lock(&client->lock);
      client->inputbuf[client->inputcount++] = buf[0];
      send(fd, buf, 1, 0);
      pthread_mutex_unlock(&client->lock);
   }
   return command;
}

void process_command(int fd, char* cmd)
{
   char *usrcmd = lower(cmd);
   char *response = handle_command(usrcmd);
   reply(fd, "", 0, response, 0);
}


/* Handle client interactions */
ClientPkg* get_client_data(int fd, fd_set *master, ServerData *serverdata)
{
   ClientPkg *clientpkg = malloc(sizeof(ClientPkg));
   if (!clientpkg)
   {
      perror("No memory for client package!");
   }

   clientpkg->nbytes = recv(fd, clientpkg->buffer, sizeof(clientpkg->buffer), 0);

   if (clientpkg->nbytes <= 0)
   {
      if (clientpkg->nbytes == 0)
      {
         // connection closed
         printf("Connection %d closed\n", fd);
      }
      else
      {
         // error
         perror("Failed to receive from socket");
      }
      close_socket(fd, master, serverdata);
   }
   return clientpkg;
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
                  process_command(i, command);
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

#endif
