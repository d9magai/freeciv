/***********************************************************************
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/

#ifdef HAVE_CONFIG_H
#include <fc_config.h>
#endif

/* common */
#include "idex.h"
#include "map.h"
#include "world_object.h"

/* threxpr */
#include "texaiplayer.h"

#include "texaiworld.h"

static struct world texai_world;

struct texai_tile_info_msg
{
  int index;
  struct terrain *terrain;
  bv_extras extras;
};

struct texai_city_info_msg
{
  int id;
  int owner;
  int tindex;
};

struct texai_id_msg
{
  int id;
};

struct texai_unit_info_msg
{
  int id;
  int owner;
  int tindex;
  int type;
};

/**************************************************************************
  Initialize world object for texai
**************************************************************************/
void texai_world_init(void)
{
  idex_init(&texai_world);
}

/**************************************************************************
  Free resources allocated for texai world object
**************************************************************************/
void texai_world_close(void)
{
  idex_free(&texai_world);
}

/**************************************************************************
  Initialize world map for texai
**************************************************************************/
void texai_map_init(void)
{
  map_init(&(texai_world.map), TRUE);
  map_allocate(&(texai_world.map));
}

/**************************************************************************
  Free resources allocated for texai world map
**************************************************************************/
void texai_map_close(void)
{
  map_free(&(texai_world.map));
}

/**************************************************************************
  Tile info updated on main map. Send update to threxpr map.
**************************************************************************/
void texai_tile_info(struct tile *ptile)
{
  if (texai_thread_running()) {
    struct texai_tile_info_msg *info = fc_malloc(sizeof(struct texai_tile_info_msg));

    info->index = tile_index(ptile);
    info->terrain = ptile->terrain;
    info->extras = ptile->extras;

    texai_send_msg(TEXAI_MSG_TILE_INFO, NULL, info);
  }
}

/**************************************************************************
  Receive tile update to the thread.
**************************************************************************/
void texai_tile_info_recv(void *data)
{
  struct texai_tile_info_msg *info = (struct texai_tile_info_msg *)data;

  if (texai_world.map.tiles != NULL) {
    struct tile *ptile;

    ptile = index_to_tile(&(texai_world.map), info->index);
    ptile->terrain = info->terrain;
    ptile->extras = info->extras;
  }

  free(info);
}

/**************************************************************************
  Send city information to the thread.
**************************************************************************/
static void texai_city_update(struct city *pcity, enum texaireqtype msgtype)
{
  struct texai_city_info_msg *info
    = fc_malloc(sizeof(struct texai_city_info_msg));

  info->id = pcity->id;
  info->owner = player_number(city_owner(pcity));
  info->tindex = tile_index(city_tile(pcity));

  texai_send_msg(msgtype, NULL, info);
}

/**************************************************************************
  New city has been added to the main map.
**************************************************************************/
void texai_city_created(struct city *pcity)
{
  if (texai_thread_running()) {
    texai_city_update(pcity, TEXAI_MSG_CITY_CREATED);
  }
}

/**************************************************************************
  Receive city update to the thread.
**************************************************************************/
void texai_city_info_recv(void *data, enum texaimsgtype msgtype)
{
  struct texai_city_info_msg *info = (struct texai_city_info_msg *)data;
  struct city *pcity;
  struct player *pplayer = player_by_number(info->owner);

  if (msgtype == TEXAI_MSG_CITY_CREATED) {
    struct tile *ptile = index_to_tile(&(texai_world.map), info->tindex);

    pcity = create_city_virtual(pplayer, ptile, "");
    pcity->id = info->id;

    idex_register_city(&texai_world, pcity);
  } else {
    pcity = idex_lookup_city(&texai_world, info->id);
  }
}
  
/**************************************************************************
  City has been removed from the main map.
**************************************************************************/
void texai_city_destroyed(struct city *pcity)
{
  if (texai_thread_running()) {
    struct texai_id_msg *info = fc_malloc(sizeof(struct texai_id_msg));

    info->id = pcity->id;

    texai_send_msg(TEXAI_MSG_CITY_DESTROYED, NULL, info);
  }
}

/**************************************************************************
  Receive city destruction to the thread.
**************************************************************************/
void texai_city_destruction_recv(void *data)
{
  struct texai_id_msg *info = (struct texai_id_msg *)data;
  struct city *pcity = idex_lookup_city(&texai_world, info->id);

  idex_unregister_city(&texai_world, pcity);
  destroy_city_virtual(pcity);
}

/**************************************************************************
  Send unit information to the thread.
**************************************************************************/
static void texai_unit_update(struct unit *punit, enum texaireqtype msgtype)
{
  struct texai_unit_info_msg *info
    = fc_malloc(sizeof(struct texai_unit_info_msg));

  info->id = punit->id;
  info->owner = player_number(unit_owner(punit));
  info->tindex = tile_index(unit_tile(punit));
  info->type = utype_number(unit_type_get(punit));

  texai_send_msg(msgtype, NULL, info);
}

/**************************************************************************
  New unit has been added to the main map.
**************************************************************************/
void texai_unit_created(struct unit *punit)
{
  if (texai_thread_running()) {
    texai_unit_update(punit, TEXAI_MSG_UNIT_CREATED);
  }
}

/**************************************************************************
  Receive unit update to the thread.
**************************************************************************/
void texai_unit_info_recv(void *data, enum texaimsgtype msgtype)
{
  struct texai_unit_info_msg *info = (struct texai_unit_info_msg *)data;
  struct unit *punit;
  struct player *pplayer = player_by_number(info->owner);
  struct unit_type *type = utype_by_number(info->type);
  struct tile *ptile = index_to_tile(&(texai_world.map), info->tindex);

  if (msgtype == TEXAI_MSG_UNIT_CREATED) {
    punit = unit_virtual_create(pplayer, NULL, type, 0);
    punit->id = info->id;

    idex_register_unit(&texai_world, punit);
  } else {
    punit = idex_lookup_unit(&texai_world, info->id);
  }

  punit->tile = ptile;
}

/**************************************************************************
  Unit has been removed from the main map.
**************************************************************************/
void texai_unit_destroyed(struct unit *punit)
{
  if (texai_thread_running()) {
    struct texai_id_msg *info = fc_malloc(sizeof(struct texai_id_msg));

    info->id = punit->id;

    texai_send_msg(TEXAI_MSG_UNIT_DESTROYED, NULL, info);
  }
}

/**************************************************************************
  Receive unit destruction to the thread.
**************************************************************************/
void texai_unit_destruction_recv(void *data)
{
  struct texai_id_msg *info = (struct texai_id_msg *)data;
  struct unit *punit = idex_lookup_unit(&texai_world, info->id);

  idex_unregister_unit(&texai_world, punit);
  unit_virtual_destroy(punit);
}
