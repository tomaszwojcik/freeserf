/*
 * map.h - Map generators and map update functions
 *
 * Copyright (C) 2012  Jon Lund Steffensen <jonlst@gmail.com>
 *
 * This file is part of freeserf.
 *
 * freeserf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * freeserf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freeserf.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAP_H
#define _MAP_H

#include <stdlib.h>
#include <stdint.h>

#include "misc.h"

/* Extract col and row from map_pos_t */
#define MAP_POS_COL(pos)  ((pos) & globals.map.col_mask)
#define MAP_POS_ROW(pos)  (((pos)>>globals.map.row_shift) & globals.map.row_mask)

/* Translate col, row coordinate to map_pos_t value. */
#define MAP_POS(x,y)  (((y)<<globals.map.row_shift) | (x))

/* Addition of two map positions. */
#define MAP_POS_ADD(pos,off)  MAP_POS(((MAP_POS_COL(pos) + MAP_POS_COL(off)) & globals.map.col_mask), \
				      ((MAP_POS_ROW(pos) + MAP_POS_ROW(off)) & globals.map.row_mask))

/* Movement of map position according to directions. */
#define MAP_MOVE(pos,dir)  MAP_POS_ADD((pos), globals.map.dirs[(dir)])

#define MAP_MOVE_RIGHT(pos)  MAP_MOVE((pos), DIR_RIGHT)
#define MAP_MOVE_DOWN_RIGHT(pos)  MAP_MOVE((pos), DIR_DOWN_RIGHT)
#define MAP_MOVE_DOWN(pos)  MAP_MOVE((pos), DIR_DOWN)
#define MAP_MOVE_LEFT(pos)  MAP_MOVE((pos), DIR_LEFT)
#define MAP_MOVE_UP_LEFT(pos)  MAP_MOVE((pos), DIR_UP_LEFT)
#define MAP_MOVE_UP(pos)  MAP_MOVE((pos), DIR_UP)

#define MAP_MOVE_UP_RIGHT(pos)  MAP_MOVE((pos), DIR_UP_RIGHT)
#define MAP_MOVE_DOWN_LEFT(pos)  MAP_MOVE((pos), DIR_DOWN_LEFT)

#define MAP_MOVE_RIGHT_N(pos,n)  MAP_POS_ADD((pos), globals.map.dirs[DIR_RIGHT]*(n))
#define MAP_MOVE_DOWN_N(pos,n)  MAP_POS_ADD((pos), globals.map.dirs[DIR_DOWN]*(n))


/* Extractors for map data. */
#define MAP_HAS_FLAG(pos)  ((uint)((globals.map.tiles[(pos)].flags >> 7) & 1))

/* This bit is used to indicate a band of positions in the water, that are
   entirely surrounded by water tiles, but are still close to the shore. This is
   the area where fish are present, and it also happens to be the only area where
   a sailor can appear as idle (since flags have to be on the shore), so it is
   used to indicate whether an idle serf should be drawn as a sailor in the viewport.
   Further, it is used to indicate on land certain positions that are impassable.*/
/* TODO Clean up; this bit has too many different meanings. */
#define MAP_DEEP_WATER(pos)  ((uint)((globals.map.tiles[(pos)].flags >> 6) & 1))

#define MAP_PATHS(pos)  ((uint)(globals.map.tiles[(pos)].flags & 0x3f))

#define MAP_HAS_OWNER(pos)  ((uint)((globals.map.tiles[(pos)].height >> 7) & 1))
#define MAP_OWNER(pos)  ((uint)((globals.map.tiles[(pos)].height >> 5) & 3))
#define MAP_HEIGHT(pos)  ((uint)(globals.map.tiles[(pos)].height & 0x1f))

#define MAP_TYPE_UP(pos)  ((uint)((globals.map.tiles[(pos)].type >> 4) & 0xf))
#define MAP_TYPE_DOWN(pos)  ((uint)(globals.map.tiles[(pos)].type & 0xf))

#define MAP_OBJ(pos)  ((map_obj_t)(globals.map.tiles[(pos)].obj & 0x7f))

/* Whether any of the two up/down tiles at this pos are water.
   This is used to indicate whether waves should be drawn. */
#define MAP_WATER(pos)  ((uint)((globals.map.tiles[(pos)].obj >> 7) & 1))

#define MAP_OBJ_INDEX(pos)  ((uint)globals.map.tiles[(pos)].u.index)
#define MAP_IDLE_SERF(pos)  ((uint)((globals.map.tiles[(pos)].u.s.field_1 >> 7) & 1))
#define MAP_PLAYER(pos)  ((uint)(globals.map.tiles[(pos)].u.s.field_1 & 3))
#define MAP_RES_TYPE(pos)  ((ground_deposit_t)((globals.map.tiles[(pos)].u.s.resource >> 5) & 7))
#define MAP_RES_AMOUNT(pos)  ((uint)(globals.map.tiles[(pos)].u.s.resource & 0x1f))
#define MAP_RES_FISH(pos)  ((uint)globals.map.tiles[(pos)].u.s.resource)
#define MAP_SERF_INDEX(pos)  ((uint)globals.map.tiles[(pos)].serf_index)


typedef enum {
	MAP_OBJ_NONE = 0,
	MAP_OBJ_FLAG,
	MAP_OBJ_SMALL_BUILDING,
	MAP_OBJ_LARGE_BUILDING,
	MAP_OBJ_CASTLE,

	MAP_OBJ_TREE_0 = 8,
	MAP_OBJ_TREE_1,
	MAP_OBJ_TREE_2, /* 10 */
	MAP_OBJ_TREE_3,
	MAP_OBJ_TREE_4,
	MAP_OBJ_TREE_5,
	MAP_OBJ_TREE_6,
	MAP_OBJ_TREE_7, /* 15 */

	MAP_OBJ_PINE_0,
	MAP_OBJ_PINE_1,
	MAP_OBJ_PINE_2,
	MAP_OBJ_PINE_3,
	MAP_OBJ_PINE_4, /* 20 */
	MAP_OBJ_PINE_5,
	MAP_OBJ_PINE_6,
	MAP_OBJ_PINE_7,

	MAP_OBJ_PALM_0,
	MAP_OBJ_PALM_1, /* 25 */
	MAP_OBJ_PALM_2,
	MAP_OBJ_PALM_3,

	MAP_OBJ_WATER_TREE_0,
	MAP_OBJ_WATER_TREE_1,
	MAP_OBJ_WATER_TREE_2, /* 30 */
	MAP_OBJ_WATER_TREE_3,

	MAP_OBJ_STONE_0 = 72,
	MAP_OBJ_STONE_1,
	MAP_OBJ_STONE_2,
	MAP_OBJ_STONE_3, /* 75 */
	MAP_OBJ_STONE_4,
	MAP_OBJ_STONE_5,
	MAP_OBJ_STONE_6,
	MAP_OBJ_STONE_7,

	MAP_OBJ_SANDSTONE_0, /* 80 */
	MAP_OBJ_SANDSTONE_1,

	MAP_OBJ_CROSS,
	MAP_OBJ_STUB,

	MAP_OBJ_STONE,
	MAP_OBJ_SANDSTONE_3, /* 85 */

	MAP_OBJ_CADAVER_0,
	MAP_OBJ_CADAVER_1,

	MAP_OBJ_WATER_STONE_0,
	MAP_OBJ_WATER_STONE_1,

	MAP_OBJ_CACTUS_0, /* 90 */
	MAP_OBJ_CACTUS_1,

	MAP_OBJ_DEAD_TREE,

	MAP_OBJ_FELLED_PINE_0,
	MAP_OBJ_FELLED_PINE_1,
	MAP_OBJ_FELLED_PINE_2, /* 95 */
	MAP_OBJ_FELLED_PINE_3,
	MAP_OBJ_FELLED_PINE_4,

	MAP_OBJ_FELLED_TREE_0,
	MAP_OBJ_FELLED_TREE_1,
	MAP_OBJ_FELLED_TREE_2, /* 100 */
	MAP_OBJ_FELLED_TREE_3,
	MAP_OBJ_FELLED_TREE_4,

	MAP_OBJ_NEW_PINE,
	MAP_OBJ_NEW_TREE,

	MAP_OBJ_SEEDS_0, /* 105 */
	MAP_OBJ_SEEDS_1,
	MAP_OBJ_SEEDS_2,
	MAP_OBJ_SEEDS_3,
	MAP_OBJ_SEEDS_4,
	MAP_OBJ_SEEDS_5, /* 110 */
	MAP_OBJ_FIELD_EXPIRED,

	MAP_OBJ_SIGN_LARGE_GOLD,
	MAP_OBJ_SIGN_SMALL_GOLD,
	MAP_OBJ_SIGN_LARGE_IRON,
	MAP_OBJ_SIGN_SMALL_IRON, /* 115 */
	MAP_OBJ_SIGN_LARGE_COAL,
	MAP_OBJ_SIGN_SMALL_COAL,
	MAP_OBJ_SIGN_LARGE_STONE,
	MAP_OBJ_SIGN_SMALL_STONE,

	MAP_OBJ_SIGN_EMPTY, /* 120 */

	MAP_OBJ_FIELD_0,
	MAP_OBJ_FIELD_1,
	MAP_OBJ_FIELD_2,
	MAP_OBJ_FIELD_3,
	MAP_OBJ_FIELD_4, /* 125 */
	MAP_OBJ_FIELD_5,
	MAP_OBJ_127
} map_obj_t;


/* A map vertex can be OPEN which means that
   a building can be constructed in the space.
   A FILLED space can be passed by a serf, but
   nothing can be built in this space. The higher
   space classes can neither be used for contructions
   nor passed by serfs. */
typedef enum {
	MAP_SPACE_OPEN = 0,
	MAP_SPACE_FILLED,
	MAP_SPACE_IMPASSABLE,
	MAP_SPACE_FLAG,
	MAP_SPACE_SMALL_BUILDING,
	MAP_SPACE_LARGE_BUILDING,
	MAP_SPACE_CASTLE
} map_space_t;

typedef enum {
	GROUND_DEPOSIT_NONE = 0,
	GROUND_DEPOSIT_GOLD,
	GROUND_DEPOSIT_IRON,
	GROUND_DEPOSIT_COAL,
	GROUND_DEPOSIT_STONE,
} ground_deposit_t;

typedef struct {
	uint8_t flags;
	uint8_t height;
	uint8_t type;
	uint8_t obj;
	union {
		uint16_t index;
		struct {
			uint8_t resource;
			uint8_t field_1;
		} s;
	} u;
	uint16_t serf_index;
} map_tile_t;


/* map_pos_t is a compact composition of col and row values that
   uniquely identifies a vertex in the map space. It is also used
   directly as index to map data arrays. */
typedef uint map_pos_t;

typedef struct {
	/* Fundamentals */
	map_tile_t *tiles;
	uint col_size, row_size;

	/* Derived */
	map_pos_t dirs[8];
	uint tile_count;
	uint cols, rows;
	uint col_mask, row_mask;
	uint row_shift;
} map_t;


/* Mapping from map_obj_t to map_space_t. */
extern const map_space_t map_space_from_obj[128];


void map_set_height(map_pos_t pos, int height);
void map_set_object(map_pos_t pos, map_obj_t obj, int index);
void map_remove_ground_deposit(map_pos_t pos, int amount);
void map_remove_fish(map_pos_t pos, int amount);
void map_set_serf_index(map_pos_t pos, int index);

int map_is_deep_water(map_pos_t pos);

void map_init_dimensions(map_t *map);
void map_init_minimap();

void map_init();
void map_update();


#endif /* _MAP_H */
