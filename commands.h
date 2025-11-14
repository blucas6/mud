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

#endif
