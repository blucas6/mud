#ifndef GAME_H
#define GAME_H

#include "room.h"

typedef struct {
   int health;
   int money;
   int level;
   char name[20];
   struct Room *croom;
} PlayerData;

typedef struct {
   PlayerData players[10];
   pthread_mutex_t lock;
} ActivePlayers;

void new_player_init(PlayerData *pd)
{
   pd->health = 10;
   pd->money = 0;
   pd->level = 1;
   strcpy(pd->name,"Ben");
}

void activeplayers_init(ActivePlayers *ap)
{
   pthread_mutex_init(&ap->lock, NULL);
}

char* move_room(char *dir, PlayerData *pd)
{
   char *buffer = malloc(ROOMINTRO);
   if (strcmp(dir, "north") == 0)
   {
      if (pd->croom->north != NULL)
      {
         snprintf(buffer, ROOMINTRO, "%s", pd->croom->north->intro);
         pd->croom = pd->croom->north;
         return buffer;
      }
   }
   else if (strcmp(dir, "south") == 0)
   {
      if (pd->croom->south != NULL)
      {
         snprintf(buffer, ROOMINTRO, "%s", pd->croom->south->intro);
         pd->croom = pd->croom->south;
         return buffer;
      }
   }
   else if (strcmp(dir, "east") == 0)
   {
      if (pd->croom->east != NULL)
      {
         snprintf(buffer, ROOMINTRO, "%s", pd->croom->east->intro);
         pd->croom = pd->croom->east;
         return buffer;
      }
   }
   else if (strcmp(dir, "west") == 0)
   {
      if (pd->croom->west != NULL)
      {
         snprintf(buffer, ROOMINTRO, "%s", pd->croom->west->intro);
         pd->croom = pd->croom->west;
         return buffer;
      }
   }
   snprintf(buffer, ROOMINTRO, "There is nothing that way.");
   return buffer;
}

#endif
