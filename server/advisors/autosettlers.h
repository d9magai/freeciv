/********************************************************************** 
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
#ifndef FC__AUTOSETTLERS_H
#define FC__AUTOSETTLERS_H

/* common */
#include "fc_types.h"
#include "map.h"

void advisors_init(void);

struct settlermap;
struct pf_path;

void adv_settlers_free(void);

void auto_settlers_player(struct player *pplayer);

void auto_settler_findwork(struct player *pplayer, 
                           struct unit *punit,
                           struct settlermap *state,
                           int recursion);

bool auto_settler_setup_work(struct player *pplayer, struct unit *punit,
                             struct settlermap *state, int recursion,
                             struct pf_path *path,
                             struct tile *best_tile,
                             enum unit_activity best_act,
                             struct extra_type **best_target,
                             int completion_time);

int settler_evaluate_improvements(struct unit *punit,
                                  enum unit_activity *best_act,
                                  struct extra_type **best_target,
                                  struct tile **best_tile,
                                  struct pf_path **path,
                                  struct settlermap *state);

struct city *settler_evaluate_city_requests(struct unit *punit,
                                            struct worker_task **best_task,
                                            struct pf_path **path,
                                            struct settlermap *state);

void adv_unit_new_task(struct unit *punit, enum adv_unit_task task,
                       struct tile *ptile);

bool adv_settler_safe_tile(const struct player *pplayer, struct unit *punit,
                           struct tile *ptile);

int adv_settlers_road_bonus(struct tile *ptile, struct road_type *proad);

extern Activity_type_id as_activities_transform[ACTIVITY_LAST];

#define as_transform_activity_iterate(_act_)                                \
{                                                                           \
  activity_type_list_iterate(as_activities_transform, _act_)

#define as_transform_activity_iterate_end                                   \
  activity_type_list_iterate_end                                            \
}

extern Activity_type_id as_activities_extra[ACTIVITY_LAST];

#define as_extra_activity_iterate(_act_)                                    \
{                                                                           \
  activity_type_list_iterate(as_activities_extra, _act_)

#define as_extra_activity_iterate_end                                       \
  activity_type_list_iterate_end                                            \
}

extern Activity_type_id as_activities_rmextra[ACTIVITY_LAST];

#define as_rmextra_activity_iterate(_act_)                                  \
{                                                                           \
  activity_type_list_iterate(as_activities_rmextra, _act_)

#define as_rmextra_activity_iterate_end                                     \
  activity_type_list_iterate_end                                            \
}

#endif   /* FC__AUTOSETTLERS_H */
