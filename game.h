#ifndef GAME_H
#define GAME_H

typedef struct {
   int health;
   int money;
   int level;
   char name[20];
} PlayerData;

typedef struct {
   PlayerData players[10];
   int count;
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
   ap->count = 0;
}

#endif
