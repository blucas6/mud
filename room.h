#ifndef ROOM_H
#define ROOM_H

#define ROOMNAME 30

struct Room {
   char name[ROOMNAME];
   char intro[200];
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
   rnorth->south = NULL;
   rnorth->east = NULL;
   rnorth->west = NULL;
   startroom->north = rnorth;
}

char* get_nearby_rooms(struct Room *room)
{
   size_t buffsz = 500;
   char *buffer = malloc(buffsz);
   char *nearby = "Current Room: %s\r\n\r\n";
   int used = snprintf(buffer, buffsz, nearby, room->name);
   
   if (room->north != NULL)
   {
      char *north_l = "North: %s\r\n";
      used = snprintf(buffer+used, buffsz-used, north_l, room->north->name);
   }
   if (room->south != NULL)
   {
      char *south_l = "South: %s\r\n";
      used = snprintf(buffer+used, buffsz-used, south_l, room->south->name);
   }
   if (room->east != NULL)
   {
      char *east_l = "East: %s\r\n";
      used = snprintf(buffer+used, buffsz-used, east_l, room->east->name);
   }
   if (room->west != NULL)
   {
      char *west_l = "East: %s\r\n";
      used = snprintf(buffer+used, buffsz-used, west_l, room->west->name);
   }

   return buffer;
}

#endif
