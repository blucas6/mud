#ifndef ROOM_H
#define ROOM_H

#define ROOMNAME 30
#define ROOMINTRO 200

struct Room {
   char name[ROOMNAME];
   char intro[ROOMINTRO];
   struct Room *north;
   struct Room *south;
   struct Room *east;
   struct Room *west;
};

void map_init(struct Room *startroom)
{
   // start room
   snprintf(startroom->name, sizeof(startroom->name), "Town Square");
   snprintf(startroom->intro, sizeof(startroom->intro), "You are at the town square.");
   startroom->north = NULL;
   startroom->south = NULL;
   startroom->east = NULL;
   startroom->west = NULL;
   // north
   struct Room *rnorth = malloc(sizeof(struct Room));
   snprintf(rnorth->name, sizeof(rnorth->name), "Cobble Stone Path");
   snprintf(rnorth->intro, sizeof(rnorth->intro),
         "You are on a cobble stone path. Grass is growing between the cobble stones.");
   rnorth->north = NULL;
   rnorth->south = startroom;
   rnorth->east = NULL;
   rnorth->west = NULL;

   startroom->north = rnorth;
   // south
   struct Room *rsouth = malloc(sizeof(struct Room));
   snprintf(rsouth->name, sizeof(rsouth->name), "Quiet Inn");
   snprintf(rsouth->intro, sizeof(rsouth->intro), 
         "You enter the inn, there is a soft candle lighting the entrance."
          " Floorboards creak as you walk around.");
   rsouth->north = startroom;
   rsouth->south = NULL;
   rsouth->east = NULL;
   rsouth->west = NULL;

   startroom->south = rsouth;

}

char* get_nearby_rooms(struct Room *room)
{
   size_t buffsz = 500;
   char *buffer = malloc(buffsz);
   char *nearby = "Current Room: %s\n\r\n\r";
   int used = snprintf(buffer, buffsz, nearby, room->name);
   
   if (room->north != NULL)
   {
      char *north_l = "North: %s\n\r";
      int u = snprintf(buffer+used, buffsz-used, north_l, room->north->name);
      used += u;
   }
   if (room->south != NULL)
   {
      char *south_l = "South: %s\n\r";
      int u = snprintf(buffer+used, buffsz-used, south_l, room->south->name);
      used += u;
   }
   if (room->east != NULL)
   {
      char *east_l = "East: %s\n\r";
      int u = snprintf(buffer+used, buffsz-used, east_l, room->east->name);
      used += u;
   }
   if (room->west != NULL)
   {
      char *west_l = "West: %s\n\r";
      int u = snprintf(buffer+used, buffsz-used, west_l, room->west->name);
      used += u;
   }

   return buffer;
}

#endif
