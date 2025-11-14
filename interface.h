#ifndef INTERFACE_H
#define INTERFACE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_W 250
#define MAX_H 80

#define ESCAPE "\033"
#define CLEAR_S "\033[2J"
#define CLEAR_SRL "\033[3J"
#define ALT "\033[?1049h"

const char* get_clear_code(void)
{
   char *res = malloc(strlen(CLEAR_S)+strlen(CLEAR_SRL)+1);
   strcpy(res, CLEAR_S);
   strcat(res, CLEAR_SRL);
   return res;
}

const char* get_alt_buffer_code(void)
{
   char *res = malloc(strlen(ALT)+1);
   strcpy(res, ALT);
   return res;
}

void update_screen(char screen[MAX_H][MAX_W], int curr_w, int curr_h)
{
   int start_h = 1;
   for (int h=start_h; h<curr_h; h++)
   {
      for (int w=0; w<curr_w; w++)
      {
         if (h == start_h || h == curr_h)
            screen[h][w] = '-';
         else if (w == 0 || w == curr_w-1)
            screen[h][w] = '|';
         else
            screen[h][w] = ' ';
      }
   }
}

/*
int save(int fd, char screen[MAX_H][MAX_W], ServerData *serverdata)
{
   pthread_mutex_lock(&serverdata->clientdata[fd].lock);
   int cl_w = serverdata->clientdata[fd].width;
   int cl_h = serverdata->clientdata[fd].height;
   pthread_mutex_unlock(&serverdata->clientdata[fd].lock);
   printf("Sending to client %d [%dx%d]\n", fd, cl_w, cl_h);
   update_screen(screen, cl_w, cl_h);
   const char *clear = get_alt_buffer_code();
   //const char *movemouse = get_move_cursor_code(cl_h-1, 3);
   size_t bufsz = ((cl_w+2) * (cl_h+1)) + strlen(clear);

   char *buf = malloc(bufsz);
   if (!buf)
   {
      perror("Failed to allocate buffer");
      return 1;
   }

   int pos = 0;

   memcpy(buf, clear, strlen(clear));
   pos += cl_w;
   buf[pos++] = '\r';
   buf[pos++] = '\n';
   for (int i=0; i<cl_h; i++)
   {
      memcpy(buf+pos, screen[i], cl_w);
      pos += cl_w;
      buf[pos++] = '\r';
      buf[pos++] = '\n';
   }
   buf[pos] = '\0';

   if (send(fd, buf, bufsz, 0) == -1)
   {
      perror("Reply failed");
      return 1;
   }
   free(buf);
   return 0;
}
*/

#endif
