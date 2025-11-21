#ifndef ROOM_H
#define ROOM_H

#include <libxml/parser.h>
#include <libxml/tree.h>

#define MAP_FILE "map.xml"
#define ROOMNAME 30
#define ROOMINTRO 200

struct Room {
   int id;
   char name[ROOMNAME];
   char intro[ROOMINTRO];
   struct Room *north;
   struct Room *south;
   struct Room *east;
   struct Room *west;
};

void print_room(struct Room *room)
{
   printf("Room:\n");
   printf("  ID: %d\n", room->id);
   printf("  Name: %s\n", room->name);
   printf("  Intro: %s\n", room->intro);
   if (room->north != NULL) printf("  North: %d\n", room->north->id);
   if (room->south != NULL) printf("  South: %d\n", room->south->id);
   if (room->east != NULL) printf("  East: %d\n", room->east->id);
   if (room->west != NULL) printf("  West: %d\n", room->west->id);
}

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

struct Room* map_load_file()
{
   xmlDocPtr fptr = xmlReadFile(MAP_FILE, NULL, 0);
   if (!fptr)
   {
      perror("Failed to open map file");
      return NULL;
   }

   struct Room *rooms = malloc(100 * sizeof(struct Room));
   if (rooms == NULL)
   {
      perror("Failed to create room pointer");
      return NULL;
   }
   int count = 0;

   xmlNode *root = xmlDocGetRootElement(fptr);
   for (xmlNode *cur = root->children; cur; cur = cur->next)
   {
      if (cur->type != XML_ELEMENT_NODE) continue;
      rooms[count].id = count;
      rooms[count].north = NULL;
      rooms[count].south = NULL;
      rooms[count].east = NULL;
      rooms[count].west = NULL;
      for (xmlNode *info = cur->children; info; info = info->next)
      {
         if (info->type != XML_ELEMENT_NODE) continue;
         xmlChar *prop = xmlNodeGetContent(info);
         if (strcmp((char*)info->name, "name") == 0)
            strcpy(rooms[count].name, (char*)prop);
         else if (strcmp((char*)info->name, "intro") == 0)
            strcpy(rooms[count].intro, (char*)prop);
         else if (strcmp((char*)info->name, "north") == 0)
            rooms[count].north = &rooms[atoi((char*)prop)];
         else if (strcmp((char*)info->name, "south") == 0)
            rooms[count].south = &rooms[atoi((char*)prop)];
         else if (strcmp((char*)info->name, "east") == 0)
            rooms[count].east = &rooms[atoi((char*)prop)];
         else if (strcmp((char*)info->name, "west") == 0)
            rooms[count].west = &rooms[atoi((char*)prop)];
         free(prop);
      }
      count++;
   }

   for (int r=0; r<count; r++)
   {
      print_room(&rooms[r]);
   }

   xmlFreeDoc(fptr);
   xmlCleanupParser();
   return &rooms[0];
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
