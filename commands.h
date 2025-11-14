#ifndef COMMANDS_H
#define COMMANDS_H

#include <string.h>
#include <ctype.h>

char* lower(char *cmd)
{
   size_t sz = strlen(cmd);
   char *lcmd = malloc(sz+1);
   for (size_t i=0; i<sz; i++)
   {
      lcmd[i] = tolower(cmd[i]);
   }
   lcmd[sz] = '\0';
   return lcmd;
}

char* handle_command(char* usrcmd)
{
   int res_size = 256;
   char *response = malloc(res_size);
   char *syscmd;

   syscmd = "hello";
   if (strcmp(usrcmd, syscmd) == 0)
   {
      response = "Hello, there!";
   }
   else
   {
      response = "Hm, I don't know that one.";
   }
   free(usrcmd);
   return response;
}

#endif
