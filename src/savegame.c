/*
 * savegame.c - Loading and saving of save games
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "map.h"
#include "globals.h"
#include "version.h"
#include "list.h"
#include "debug.h"


typedef struct {
	uint rows, cols;
	uint row_shift;
} v0_map_t;

static map_pos_t
load_v0_map_pos(const v0_map_t *map, uint32_t value)
{
	return MAP_POS((value >> 2) & (map->cols-1),
		       (value >> (2 + map->row_shift)) & (map->rows-1));
}

/* Load global state from save game. */
static int
load_v0_globals_state(FILE *f, v0_map_t *map)
{
	uint8_t *data = malloc(210);
	if (data == NULL) return -1;

	size_t rd = fread(data, sizeof(uint8_t), 210, f);
	if (rd < 210) {
		free(data);
		return -1;
	}

	/* Load these first so map dimensions can be reconstructed.
	   This is necessary to load map positions. */
	globals.map_size = *(uint16_t *)&data[190];

	map->row_shift = *(uint16_t *)&data[42];
	map->cols = *(uint16_t *)&data[62];
	map->rows = *(uint16_t *)&data[64];

	/* Init the rest of map dimensions. */
	globals.map.col_size = 5 + globals.map_size/2;
	globals.map.row_size = 5 + (globals.map_size - 1)/2;
	globals.map.cols = 1 << globals.map.col_size;
	globals.map.rows = 1 << globals.map.row_size;
	map_init_dimensions(&globals.map);

	/* OBSOLETE may be needed to load map data correctly?
	map->index_mask = *(uint32_t *)&data[0] >> 2;

	globals.map_dirs[DIR_RIGHT] = *(uint32_t *)&data[4] >> 2;
	globals.map_dirs[DIR_DOWN_RIGHT] = *(uint32_t *)&data[8] >> 2;
	globals.map_dirs[DIR_DOWN] = *(uint32_t *)&data[12] >> 2;
	globals.map_move_left_2 = *(uint16_t *)&data[16] >> 2;
	globals.map_dirs[DIR_UP_LEFT] = *(uint32_t *)&data[18] >> 2;
	globals.map_dirs[DIR_UP] = *(uint32_t *)&data[22] >> 2;
	globals.map_dirs[DIR_UP_RIGHT] = *(uint32_t *)&data[26] >> 2;
	globals.map_dirs[DIR_DOWN_LEFT] = *(uint32_t *)&data[30] >> 2;

	globals.map_col_size = *(uint32_t *)&data[34] >> 2;
	globals.map_elms = *(uint32_t *)&data[38];
	globals.map_row_shift = *(uint16_t *)&data[42];
	globals.map_col_mask = *(uint16_t *)&data[44];
	globals.map_row_mask = *(uint16_t *)&data[46];
	globals.map_data_offset = *(uint32_t *)&data[48] >> 2;
	globals.map_shifted_col_mask = *(uint16_t *)&data[52] >> 2;
	globals.map_shifted_row_mask = *(uint32_t *)&data[54] >> 2;
	globals.map_col_pairs = *(uint16_t *)&data[58];
	globals.map_row_pairs = *(uint16_t *)&data[60];*/

	globals.split = *(uint8_t *)&data[66];
	/* globals.field_37F = *(uint8_t *)&data[67]; */
	globals.update_map_initial_pos = load_v0_map_pos(map, *(uint32_t *)&data[68]);

	globals.cfg_left = *(uint8_t *)&data[72];
	globals.cfg_right = *(uint8_t *)&data[73];

	globals.game_type = *(uint16_t *)&data[74];
	globals.game_tick = *(uint32_t *)&data[76];
	globals.game_stats_counter = *(uint16_t *)&data[80];
	globals.history_counter = *(uint16_t *)&data[82];

	globals.rnd.state[0] = *(uint16_t *)&data[84];
	globals.rnd.state[1] = *(uint16_t *)&data[86];
	globals.rnd.state[2] = *(uint16_t *)&data[88];

	globals.max_ever_flag_index = *(uint16_t *)&data[90];
	globals.max_ever_building_index = *(uint16_t *)&data[92];
	globals.max_ever_serf_index = *(uint16_t *)&data[94];

	globals.next_index = *(uint16_t *)&data[96];
	globals.flag_search_counter = *(uint16_t *)&data[98];
	globals.update_map_last_anim = *(uint16_t *)&data[100];
	globals.update_map_counter = *(uint16_t *)&data[102];

	for (int i = 0; i < 4; i++) {
		globals.player_history_index[i] = *(uint16_t *)&data[104 + i*2];
	}

	for (int i = 0; i < 3; i++) {
		globals.player_history_counter[i] = *(uint16_t *)&data[112 + i*2];
	}

	globals.resource_history_index = *(uint16_t *)&data[118];

	globals.map_regions = *(uint16_t *)&data[120];

	if (0/*globals.game_type == GAME_TYPE_TUTORIAL*/) {
		globals.tutorial_level = *(uint16_t *)&data[122];
	} else if (0/*globals.game_type == GAME_TYPE_MISSION*/) {
		globals.mission_level = *(uint16_t *)&data[124];
		/*globals.max_mission_level = *(uint16_t *)&data[126];*/
		/* memcpy(globals.mission_code, &data[128], 8); */
	} else if (1/*globals.game_type == GAME_TYPE_1_PLAYER*/) {
		/*globals.menu_map_size = *(uint16_t *)&data[136];*/
		/*globals.rnd_init_1 = *(uint16_t *)&data[138];
		globals.rnd_init_2 = *(uint16_t *)&data[140];
		globals.rnd_init_3 = *(uint16_t *)&data[142];*/

		/*
		memcpy(globals.menu_ai_face, &data[144], 4);
		memcpy(globals.menu_ai_intelligence, &data[148], 4);
		memcpy(globals.menu_ai_supplies, &data[152], 4);
		memcpy(globals.menu_ai_reproduction, &data[156], 4);
		*/

		/*
		memcpy(globals.menu_human_supplies, &data[160], 2);
		memcpy(globals.menu_human_reproduction, &data[162], 2);
		*/
	}

	/*
	globals.saved_pl1_map_cursor_col = *(uint16_t *)&data[164];
	globals.saved_pl1_map_cursor_row = *(uint16_t *)&data[166];

	globals.saved_pl1_pl_sett_100 = *(uint8_t *)&data[168];
	globals.saved_pl1_pl_sett_101 = *(uint8_t *)&data[169];
	globals.saved_pl1_pl_sett_102 = *(uint16_t *)&data[170];

	globals.saved_pl1_build = *(uint8_t *)&data[172];
	globals.field_17B = *(uint8_t *)&data[173];
	*/

	globals.max_ever_inventory_index = *(uint16_t *)&data[174];
	globals.map_max_serfs_left = *(uint16_t *)&data[176];
	/* globals.max_stock_buildings = *(uint16_t *)&data[178]; */
	globals.max_next_index = *(uint16_t *)&data[180];
	globals.map_field_4A = *(uint16_t *)&data[182];
	globals.map_gold_deposit = *(uint32_t *)&data[184];
	globals.update_map_16_loop = *(uint16_t *)&data[188];

	globals.map_field_52 = *(uint16_t *)&data[192];
	/*
	globals.field_54 = *(uint16_t *)&data[194];
	globals.field_56 = *(uint16_t *)&data[196];
	*/

	globals.map_62_5_times_regions = *(uint16_t *)&data[198];
	globals.map_gold_morale_factor = *(uint16_t *)&data[200];
	globals.winning_player = *(uint16_t *)&data[202];
	globals.player_score_leader = *(uint8_t *)&data[204];
	/*
	globals.show_game_end_box = *(uint8_t *)&data[205];
	*/

	/*globals.map_dirs[DIR_LEFT] = *(uint32_t *)&data[206] >> 2;*/

	free(data);

	/* Skip unused section. */
	int r = fseek(f, 40, SEEK_CUR);
	if (r < 0) return -1;

	return 0;
}

/* Load player state from save game. */
static int
load_v0_player_sett_state(FILE *f)
{
	uint8_t *data = malloc(8628);
	if (data == NULL) return -1;

	for (int i = 0; i < 4; i++) {
		size_t rd = fread(data, sizeof(uint8_t), 8628, f);
		if (rd < 8628) {
			free(data);
			return -1;
		}

		player_sett_t *sett = globals.player_sett[i];

		for (int j = 0; j < 9; j++) {
			sett->tool_prio[j] = *(uint16_t *)&data[2*j];
		}

		for (int j = 0; j < 26; j++) {
			sett->resource_count[j] = data[18+j];
			sett->flag_prio[j] = data[44+j];
		}

		for (int j = 0; j < 27; j++) {
			sett->serf_count[j] = *(uint16_t *)&data[70+2*j];
		}

		for (int j = 0; j < 4; j++) {
			sett->knight_occupation[j] = data[124+j];
		}

		sett->player_num = *(uint16_t *)&data[128];
		sett->flags = data[130];
		sett->build = data[131];

		for (int j = 0; j < 23; j++) {
			sett->completed_building_count[j] = *(uint16_t *)&data[132+2*j];
			sett->incomplete_building_count[j] = *(uint16_t *)&data[178+2*j];
		}

		for (int j = 0; j < 26; j++) {
			sett->inventory_prio[j] = data[224+j];
		}

		for (int j = 0; j < 64; j++) {
			sett->attacking_buildings[j] = *(uint16_t *)&data[250+2*j];
		}

		sett->current_sett_5_item = *(uint16_t *)&data[378];
		sett->map_cursor_col = *(uint16_t *)&data[380];
		sett->map_cursor_row = *(uint16_t *)&data[382];
		sett->map_cursor_type = data[384];
		sett->panel_btn_type = data[385];
		sett->building_height_after_level = *(uint16_t *)&data[386];
		sett->building = *(uint16_t *)&data[388];

		sett->castle_flag = *(uint16_t *)&data[390];
		sett->castle_inventory = *(uint16_t *)&data[392];

		sett->cont_search_after_non_optimal_find = *(uint16_t *)&data[394];
		sett->knights_to_spawn = *(uint16_t *)&data[396];
		sett->field_110 = *(uint16_t *)&data[400];

		sett->total_land_area = *(uint32_t *)&data[402];
		sett->total_building_score = *(uint32_t *)&data[406];
		sett->total_military_score = *(uint32_t *)&data[410];

		sett->last_anim = *(uint16_t *)&data[414];

		sett->reproduction_counter = *(uint16_t *)&data[416];
		sett->reproduction_reset = *(uint16_t *)&data[418];
		sett->serf_to_knight_rate = *(uint16_t *)&data[420];
		sett->serf_to_knight_counter = *(uint16_t *)&data[422];

		sett->attacking_building_count = *(uint16_t *)&data[424];
		for (int j = 0; j < 4; j++) {
			sett->attacking_knights[j] = *(uint16_t *)&data[426+2*j];
		}
		sett->total_attacking_knights = *(uint16_t *)&data[434];
		sett->building_attacked = *(uint16_t *)&data[436];
		sett->knights_attacking = *(uint16_t *)&data[438];

		sett->analysis_goldore = *(uint16_t *)&data[440];
		sett->analysis_ironore = *(uint16_t *)&data[442];
		sett->analysis_coal = *(uint16_t *)&data[444];
		sett->analysis_stone = *(uint16_t *)&data[446];

		sett->food_stonemine = *(uint16_t *)&data[448];
		sett->food_coalmine = *(uint16_t *)&data[450];
		sett->food_ironmine = *(uint16_t *)&data[452];
		sett->food_goldmine = *(uint16_t *)&data[454];

		sett->planks_construction = *(uint16_t *)&data[456];
		sett->planks_boatbuilder = *(uint16_t *)&data[458];
		sett->planks_toolmaker = *(uint16_t *)&data[460];

		sett->steel_toolmaker = *(uint16_t *)&data[462];
		sett->steel_weaponsmith = *(uint16_t *)&data[464];

		sett->coal_steelsmelter = *(uint16_t *)&data[466];
		sett->coal_goldsmelter = *(uint16_t *)&data[468];
		sett->coal_weaponsmith = *(uint16_t *)&data[470];

		sett->wheat_pigfarm = *(uint16_t *)&data[472];
		sett->wheat_mill = *(uint16_t *)&data[474];

		sett->current_sett_6_item = *(uint16_t *)&data[476];

		sett->castle_score = *(int16_t *)&data[478];

		/* TODO */
	}

	free(data);

	return 0;
}

/* Load map state from save game. */
static int
load_v0_map_state(FILE *f, const v0_map_t *map)
{
	uint tile_count = map->cols*map->rows;

	uint8_t *data = malloc(8*tile_count);
	if (data == NULL) return -1;

	size_t rd = fread(data, sizeof(uint8_t), 8*tile_count, f);
	if (rd < 8*tile_count) {
		free(data);
		return -1;
	}

	map_tile_t *tiles = globals.map.tiles;

	for (int y = 0; y < globals.map.rows; y++) {
		for (int x = 0; x < globals.map.cols; x++) {
			map_pos_t pos = MAP_POS(x, y);
			uint8_t *field_1_data = &data[4*(x + (y << map->row_shift))];
			uint8_t *field_2_data = &data[4*(x + (y << map->row_shift)) + 4*map->cols];

			tiles[pos].flags = field_1_data[0];
			tiles[pos].height = field_1_data[1];
			tiles[pos].type = field_1_data[2];
			tiles[pos].obj = field_1_data[3];

			if (MAP_OBJ(pos) >= MAP_OBJ_FLAG &&
			    MAP_OBJ(pos) <= MAP_OBJ_CASTLE) {
				tiles[pos].u.index = *(uint16_t *)&field_2_data[0];
			} else {
				tiles[pos].u.s.resource = field_2_data[0];
				tiles[pos].u.s.field_1 = field_2_data[1];
			}

			tiles[pos].serf_index = *(uint16_t *)&field_2_data[2];
		}
	}

	free(data);

	return 0;
}

/* Load serf state from save game. */
static int
load_v0_serf_state(FILE *f, const v0_map_t *map)
{
	/* Load serf bitmap. */
	int bitmap_size = 4*((globals.max_ever_serf_index + 31)/32);
	uint8_t *bitmap = malloc(bitmap_size);
	if (bitmap == NULL) return -1;

	size_t rd = fread(bitmap, sizeof(uint8_t), bitmap_size, f);
	if (rd < bitmap_size) {
		free(bitmap);
		return -1;
	}

	memset(globals.serfs_bitmap, '\0', (globals.max_serf_cnt+31)/32);
	memcpy(globals.serfs_bitmap, bitmap, bitmap_size);

	free(bitmap);

	/* Load serf data. */
	uint8_t *data = malloc(16*globals.max_ever_serf_index);
	if (data == NULL) return -1;

	rd = fread(data, 16*sizeof(uint8_t), globals.max_ever_serf_index, f);
	if (rd < globals.max_ever_serf_index) {
		free(data);
		return -1;
	}

	for (int i = 0; i < globals.max_ever_serf_index; i++) {
		uint8_t *serf_data = &data[16*i];
		serf_t *serf = &globals.serfs[i];

		serf->type = serf_data[0];
		serf->animation = serf_data[1];
		serf->counter = *(uint16_t *)&serf_data[2];
		serf->pos = load_v0_map_pos(map, *(uint32_t *)&serf_data[4]);
		serf->anim = *(uint16_t *)&serf_data[8];
		serf->state = serf_data[10];

		LOGV("savegame", "load serf %i: %s", i, serf_get_state_name(serf->state));

		switch (serf->state) {
		case SERF_STATE_IDLE_IN_STOCK:
			serf->s.idle_in_stock.inv_index = *(uint16_t *)&serf_data[14];
			break;

		case SERF_STATE_WALKING:
		case SERF_STATE_TRANSPORTING:
		case SERF_STATE_DELIVERING:
			serf->s.walking.res = *(int8_t *)&serf_data[11];
			serf->s.walking.dest = *(uint16_t *)&serf_data[12];
			serf->s.walking.dir = *(int8_t *)&serf_data[14];
			serf->s.walking.wait_counter = *(int8_t *)&serf_data[15];
			break;

		case SERF_STATE_ENTERING_BUILDING:
			serf->s.entering_building.field_B = *(int8_t *)&serf_data[11];
			serf->s.entering_building.slope_len = *(uint16_t *)&serf_data[12];
			break;

		case SERF_STATE_LEAVING_BUILDING:
		case SERF_STATE_READY_TO_LEAVE:
			serf->s.leaving_building.field_B = *(int8_t *)&serf_data[11];
			serf->s.leaving_building.dest = *(int8_t *)&serf_data[12];
			serf->s.leaving_building.dest2 = *(int8_t *)&serf_data[13];
			serf->s.leaving_building.dir = *(int8_t *)&serf_data[14];
			serf->s.leaving_building.next_state = serf_data[15];
			break;

		case SERF_STATE_READY_TO_ENTER:
			serf->s.ready_to_enter.field_B = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_DIGGING:
			serf->s.digging.h_index = *(int8_t *)&serf_data[11];
			serf->s.digging.target_h = serf_data[12];
			serf->s.digging.dig_pos = *(int8_t *)&serf_data[13];
			serf->s.digging.substate = *(int8_t *)&serf_data[14];
			break;

		case SERF_STATE_BUILDING:
			serf->s.building.mode = *(int8_t *)&serf_data[11];
			serf->s.building.bld_index = *(uint16_t *)&serf_data[12];
			serf->s.building.material_step = serf_data[14];
			serf->s.building.counter = serf_data[15];
			break;

		case SERF_STATE_BUILDING_CASTLE:
			serf->s.building_castle.inv_index = *(uint16_t *)&serf_data[12];
			break;

		case SERF_STATE_MOVE_RESOURCE_OUT:
		case SERF_STATE_DROP_RESOURCE_OUT:
			serf->s.move_resource_out.res = serf_data[11];
			serf->s.move_resource_out.res_dest = *(uint16_t *)&serf_data[12];
			serf->s.move_resource_out.next_state = serf_data[15];
			break;

		case SERF_STATE_READY_TO_LEAVE_INVENTORY:
			serf->s.ready_to_leave_inventory.mode = *(int8_t *)&serf_data[11];
			serf->s.ready_to_leave_inventory.dest = *(uint16_t *)&serf_data[12];
			serf->s.ready_to_leave_inventory.inv_index = *(uint16_t *)&serf_data[14];
			break;

		case SERF_STATE_FREE_WALKING:
		case SERF_STATE_LOGGING:
		case SERF_STATE_PLANTING:
		case SERF_STATE_STONECUTTING:
		case SERF_STATE_STONECUTTER_FREE_WALKING:
		case SERF_STATE_FISHING:
		case SERF_STATE_FARMING:
		case SERF_STATE_SAMPLING_GEO_SPOT:
			serf->s.free_walking.dist1 = *(int8_t *)&serf_data[11];
			serf->s.free_walking.dist2 = *(int8_t *)&serf_data[12];
			serf->s.free_walking.neg_dist1 = *(int8_t *)&serf_data[13];
			serf->s.free_walking.neg_dist2 = *(int8_t *)&serf_data[14];
			serf->s.free_walking.flags = *(int8_t *)&serf_data[15];
			break;

		case SERF_STATE_SAWING:
			serf->s.sawing.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_LOST:
			serf->s.lost.field_B = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_MINING:
			serf->s.mining.substate = serf_data[11];
			serf->s.mining.res = serf_data[13];
			serf->s.mining.deposit = serf_data[14];
			break;

		case SERF_STATE_SMELTING:
			serf->s.smelting.mode = *(int8_t *)&serf_data[11];
			serf->s.smelting.counter = *(int8_t *)&serf_data[12];
			serf->s.smelting.type = serf_data[13];
			break;

		case SERF_STATE_MILLING:
			serf->s.milling.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_BAKING:
			serf->s.baking.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_PIGFARMING:
			serf->s.pigfarming.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_BUTCHERING:
			serf->s.butchering.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_MAKING_WEAPON:
			serf->s.making_weapon.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_MAKING_TOOL:
			serf->s.making_tool.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_BUILDING_BOAT:
			serf->s.building_boat.mode = *(int8_t *)&serf_data[11];
			break;

		case SERF_STATE_KNIGHT_DEFENDING_VICTORY_FREE:
			/* TODO This will be tricky to load since the
			   function of this state has been changed to one
			   that is driven by the attacking serf instead
			   (SERF_STATE_KNIGHT_ATTACKING_DEFEAT_FREE). */
			break;

		case SERF_STATE_IDLE_ON_PATH:
		case SERF_STATE_WAIT_IDLE_ON_PATH:
		case SERF_STATE_WAKE_AT_FLAG:
		case SERF_STATE_WAKE_ON_PATH:
			serf->s.idle_on_path.rev_dir = *(int8_t *)&serf_data[11];
			serf->s.idle_on_path.flag = &globals.flgs[*(uint32_t *)&serf_data[12]/70];
			serf->s.idle_on_path.field_E = serf_data[14];
			break;

		case SERF_STATE_DEFENDING_HUT:
		case SERF_STATE_DEFENDING_TOWER:
		case SERF_STATE_DEFENDING_FORTRESS:
		case SERF_STATE_DEFENDING_CASTLE:
			serf->s.defending.next_knight = *(uint16_t *)&serf_data[14];
			break;

		default: break;
		}
	}

	free(data);

	return 0;
}

/* Load flag state from save game. */
static int
load_v0_flag_state(FILE *f)
{
	/* Load flag bitmap. */
	int bitmap_size = 4*((globals.max_ever_flag_index + 31)/32);
	uint8_t *flag_bitmap = malloc(bitmap_size);
	if (flag_bitmap == NULL) return -1;

	size_t rd = fread(flag_bitmap, sizeof(uint8_t), bitmap_size, f);
	if (rd < bitmap_size) {
		free(flag_bitmap);
		return -1;
	}

	memset(globals.flg_bitmap, '\0', (globals.max_flg_cnt+31)/32);
	memcpy(globals.flg_bitmap, flag_bitmap, bitmap_size);

	free(flag_bitmap);

	/* Load flag data. */
	uint8_t *data = malloc(70*globals.max_ever_flag_index);
	if (data == NULL) return -1;

	rd = fread(data, 70*sizeof(uint8_t), globals.max_ever_flag_index, f);
	if (rd < globals.max_ever_flag_index) {
		free(data);
		return -1;
	}

	for (int i = 0; i < globals.max_ever_flag_index; i++) {
		uint8_t *flag_data = &data[70*i];
		flag_t *flag = &globals.flgs[i];

		flag->pos = MAP_POS(0, 0); /* TODO */
		flag->search_num = *(uint16_t *)&flag_data[0];
		flag->search_dir = flag_data[2];
		flag->path_con = flag_data[3];
		flag->endpoint = flag_data[4];
		flag->transporter = flag_data[5];

		for (int j = 0; j < 6; j++) {
			flag->length[j] = flag_data[6+j];
		}

		for (int j = 0; j < 8; j++) {
			flag->res_waiting[j] = flag_data[12+j];
			flag->res_dest[j] = *(uint16_t *)&flag_data[20+2*j];
		}

		for (int j = 0; j < 6; j++) {
			int offset = *(uint32_t *)&flag_data[36+4*j];
			flag->other_endpoint.f[j] = &globals.flgs[offset/70];

			/* Other endpoint could be a building in direction up left. */
			if (j == 4 && BIT_TEST(flag->endpoint, 6)) {
				flag->other_endpoint.b[j] = &globals.buildings[offset/18];
			}

			flag->other_end_dir[j] = flag_data[60+j];
		}

		flag->bld_flags = flag_data[66];
		flag->stock1_prio = flag_data[67];
		flag->bld2_flags = flag_data[68];
		flag->stock2_prio = flag_data[69];
	}

	free(data);

	/* Set flag positions. */
	for (int y = 0; y < globals.map.rows; y++) {
		for (int x = 0; x < globals.map.cols; x++) {
			map_pos_t pos = MAP_POS(x, y);
			if (MAP_OBJ(pos) == MAP_OBJ_FLAG) {
				flag_t *flag = game_get_flag(MAP_OBJ_INDEX(pos));
				flag->pos = pos;
			}
		}
	}

	return 0;
}

/* Load building state from save game. */
static int
load_v0_building_state(FILE *f, const v0_map_t *map)
{
	/* Load building bitmap. */
	int bitmap_size = 4*((globals.max_ever_building_index + 31)/32);
	uint8_t *bitmap = malloc(bitmap_size);
	if (bitmap == NULL) return -1;

	size_t rd = fread(bitmap, sizeof(uint8_t), bitmap_size, f);
	if (rd < bitmap_size) {
		free(bitmap);
		return -1;
	}

	memset(globals.buildings_bitmap, '\0', (globals.max_building_cnt+31)/32);
	memcpy(globals.buildings_bitmap, bitmap, bitmap_size);

	free(bitmap);

	/* Load building data. */
	uint8_t *data = malloc(18*globals.max_ever_building_index);
	if (data == NULL) return -1;

	rd = fread(data, 18*sizeof(uint8_t), globals.max_ever_building_index, f);
	if (rd < globals.max_ever_building_index) {
		free(data);
		return -1;
	}

	for (int i = 0; i < globals.max_ever_building_index; i++) {
		uint8_t *building_data = &data[18*i];
		building_t *building = &globals.buildings[i];

		building->pos = load_v0_map_pos(map, *(uint32_t *)&building_data[0]);
		building->bld = building_data[4];
		building->serf = building_data[5];
		building->flg_index = *(uint16_t *)&building_data[6];
		building->stock1 = building_data[8];
		building->stock2 = building_data[9];
		building->serf_index = *(uint16_t *)&building_data[10];
		building->progress = *(uint16_t *)&building_data[12];

		if (!BUILDING_IS_BURNING(building) &&
		    (BUILDING_IS_DONE(building) ||
		     BUILDING_TYPE(building) == BUILDING_CASTLE)) {
			int offset = *(uint32_t *)&building_data[14];
			if (BUILDING_TYPE(building) == BUILDING_STOCK ||
			    BUILDING_TYPE(building) == BUILDING_CASTLE) {
				building->u.inventory = &globals.inventories[offset/120];
			} else {
				building->u.flag = &globals.flgs[offset/70];
			}
		} else {
			building->u.s.level = *(uint16_t *)&building_data[14];
			building->u.s.planks_needed = building_data[16];
			building->u.s.stone_needed = building_data[17];
		}
	}

	free(data);

	return 0;
}

/* Load inventory state from save game. */
static int
load_v0_inventory_state(FILE *f)
{
	/* Load inventory bitmap. */
	int bitmap_size = 4*((globals.max_ever_inventory_index + 31)/32);
	uint8_t *bitmap = malloc(bitmap_size);
	if (bitmap == NULL) return -1;

	size_t rd = fread(bitmap, sizeof(uint8_t), bitmap_size, f);
	if (rd < bitmap_size) {
		free(bitmap);
		return -1;
	}

	memset(globals.inventories_bitmap, '\0', (globals.max_inventory_cnt+31)/32);
	memcpy(globals.inventories_bitmap, bitmap, bitmap_size);

	free(bitmap);

	/* Load inventory data. */
	uint8_t *data = malloc(120*globals.max_ever_inventory_index);
	if (data == NULL) return -1;

	rd = fread(data, 120*sizeof(uint8_t), globals.max_ever_inventory_index, f);
	if (rd < globals.max_ever_inventory_index) {
		free(data);
		return -1;
	}

	for (int i = 0; i < globals.max_ever_inventory_index; i++) {
		uint8_t *inventory_data = &data[120*i];
		inventory_t *inventory = &globals.inventories[i];

		inventory->player_num = inventory_data[0];
		inventory->res_dir = inventory_data[1];
		inventory->flg_index = *(uint16_t *)&inventory_data[2];
		inventory->bld_index = *(uint16_t *)&inventory_data[4];

		for (int j = 0; j < 26; j++) {
			inventory->resources[j] = *(uint16_t *)&inventory_data[6+2*j];
		}

		for (int j = 0; j < 2; j++) {
			inventory->out_queue[j] = inventory_data[58+j]-1;
			inventory->out_dest[j] = *(uint16_t *)&inventory_data[60+2*j];
		}

		inventory->spawn_priority = *(uint16_t *)&inventory_data[64];

		for (int j = 0; j < 27; j++) {
			inventory->serfs[j] = *(uint16_t *)&inventory_data[66+2*j];
		}
	}

	free(data);

	return 0;
}

/* Load a save game. */
int
load_v0_state(FILE *f)
{
	int r;
	v0_map_t map;

	r = load_v0_globals_state(f, &map);
	if (r < 0) return -1;

	r = load_v0_player_sett_state(f);
	if (r < 0) return -1;

	r = load_v0_map_state(f, &map);
	if (r < 0) return -1;

	r = load_v0_serf_state(f, &map);
	if (r < 0) return -1;

	r = load_v0_flag_state(f);
	if (r < 0) return -1;

	r = load_v0_building_state(f, &map);
	if (r < 0) return -1;

	r = load_v0_inventory_state(f);
	if (r < 0) return -1;

	globals.game_speed = 0;
	globals.game_speed_save = DEFAULT_GAME_SPEED;

	return 0;
}


static void
save_text_write_value(FILE *f, const char *name, int value)
{
	fprintf(f, "%s=%i\n", name, value);
}

static void
save_text_write_string(FILE *f, const char *name, const char *value)
{
	fprintf(f, "%s=%s\n", name, value);
}

static void
save_text_write_map_pos(FILE *f, const char *name, map_pos_t pos)
{
	fprintf(f, "%s=%u,%u\n", name,
		MAP_POS_COL(pos), MAP_POS_ROW(pos));
}

static void
save_text_write_array(FILE *f, const char *name,
		      const int values[], uint size)
{
	fprintf(f, "%s=", name);
	for (int i = 0; i < size-1; i++) fprintf(f, "%i,", values[i]);
	if (size > 0) fprintf(f, "%i", values[size-1]);
	fprintf(f, "\n");
}

static int
save_text_globals_state(FILE *f)
{
	fprintf(f, "[globals]\n");

	save_text_write_string(f, "version", FREESERF_VERSION);

	save_text_write_value(f, "map.col_size", globals.map.col_size);
	save_text_write_value(f, "map.row_size", globals.map.row_size);

	save_text_write_value(f, "split", globals.split);
	save_text_write_map_pos(f, "update_map_initial_pos", globals.update_map_initial_pos);
	save_text_write_value(f, "cfg.left", globals.cfg_left);
	save_text_write_value(f, "cfg.right", globals.cfg_right);

	save_text_write_value(f, "game_type", globals.game_type);
	save_text_write_value(f, "game_tick", globals.game_tick);
	save_text_write_value(f, "game_stats_counter", globals.game_stats_counter);
	save_text_write_value(f, "history_counter", globals.history_counter);

	int rnd[3] = { globals.rnd.state[0],
		       globals.rnd.state[1],
		       globals.rnd.state[2] };
	save_text_write_array(f, "rnd", rnd, 3);

	save_text_write_value(f, "max_ever_flag_index", globals.max_ever_flag_index);
	save_text_write_value(f, "max_ever_building_index", globals.max_ever_building_index);
	save_text_write_value(f, "max_ever_serf_index", globals.max_ever_serf_index);

	save_text_write_value(f, "next_index", globals.next_index);
	save_text_write_value(f, "flag_search_counter", globals.flag_search_counter);
	save_text_write_value(f, "update_map_last_anim", globals.update_map_last_anim);
	save_text_write_value(f, "update_map_counter", globals.update_map_counter);

	save_text_write_array(f, "player_history_index", globals.player_history_index, 4);
	save_text_write_array(f, "player_history_counter", globals.player_history_counter, 3);
	save_text_write_value(f, "resource_history_index", globals.resource_history_index);

	save_text_write_value(f, "map.regions", globals.map_regions);

	save_text_write_value(f, "max_ever_inventory_index", globals.max_ever_inventory_index);
	save_text_write_value(f, "map.max_serfs_left", globals.map_max_serfs_left);
	save_text_write_value(f, "max_next_index", globals.max_next_index);
	save_text_write_value(f, "map.field_4A", globals.map_field_4A);
	save_text_write_value(f, "map.gold_deposit", globals.map_gold_deposit);
	save_text_write_value(f, "update_map_16_loop", globals.update_map_16_loop);
	save_text_write_value(f, "map.size", globals.map_size);
	save_text_write_value(f, "map.field_52", globals.map_field_52);

	save_text_write_value(f, "map.62_5_times_regions", globals.map_62_5_times_regions);
	save_text_write_value(f, "map.gold_morale_factor", globals.map_gold_morale_factor);
	save_text_write_value(f, "winning_player", globals.winning_player);
	save_text_write_value(f, "player_score_leader", globals.player_score_leader);
	fprintf(f, "\n");

	return 0;
}

static int
save_text_player_state(FILE *f)
{
	for (int i = 0; i < 4; i++) {
		player_sett_t *sett = globals.player_sett[i];
		if (!BIT_TEST(globals.player_sett[i]->flags, 6)) continue; /* Not active */

		fprintf(f, "[player %i]\n", i);

		save_text_write_value(f, "flags", sett->flags);
		save_text_write_value(f, "build", sett->build);

		save_text_write_array(f, "tool_prio", sett->tool_prio, 9);
		save_text_write_array(f, "resource_count", sett->resource_count, 26);
		save_text_write_array(f, "flag_prio", sett->flag_prio, 26);
		save_text_write_array(f, "serf_count", sett->serf_count, 27);
		save_text_write_array(f, "knight_occupation", sett->knight_occupation, 4);

		save_text_write_array(f, "completed_building_count", sett->completed_building_count, 23);
		save_text_write_array(f, "incomplete_building_count", sett->incomplete_building_count, 23);

		save_text_write_array(f, "inventory_prio", sett->inventory_prio, 26);
		save_text_write_array(f, "attacking_buildings", sett->attacking_buildings, 64);

		save_text_write_value(f, "map_cursor.col", sett->map_cursor_col);
		save_text_write_value(f, "map_cursor.row", sett->map_cursor_row);

		save_text_write_value(f, "knights_to_spawn", sett->knights_to_spawn);
		save_text_write_value(f, "last_anim", sett->last_anim);

		save_text_write_value(f, "reproduction_counter", sett->reproduction_counter);
		save_text_write_value(f, "reproduction_reset", sett->reproduction_reset);
		save_text_write_value(f, "serf_to_knight_rate", sett->serf_to_knight_rate);
		save_text_write_value(f, "serf_to_knight_counter", sett->serf_to_knight_counter);

		save_text_write_value(f, "attacking_building_count", sett->attacking_building_count);
		save_text_write_array(f, "attacking_knights", sett->attacking_knights, 4);
		save_text_write_value(f, "total_attacking_knights", sett->total_attacking_knights);
		save_text_write_value(f, "building_attacked", sett->building_attacked);
		save_text_write_value(f, "knights_attacking", sett->knights_attacking);

		save_text_write_value(f, "food_stonemine", sett->food_stonemine);
		save_text_write_value(f, "food_coalmine", sett->food_coalmine);
		save_text_write_value(f, "food_ironmine", sett->food_ironmine);
		save_text_write_value(f, "food_goldmine", sett->food_goldmine);

		save_text_write_value(f, "planks_construction", sett->planks_construction);
		save_text_write_value(f, "planks_boatbuilder", sett->planks_boatbuilder);
		save_text_write_value(f, "planks_toolmaker", sett->planks_toolmaker);

		save_text_write_value(f, "steel_toolmaker", sett->steel_toolmaker);
		save_text_write_value(f, "steel_weaponsmith", sett->steel_weaponsmith);

		save_text_write_value(f, "coal_steelsmelter", sett->coal_steelsmelter);
		save_text_write_value(f, "coal_goldsmelter", sett->coal_goldsmelter);
		save_text_write_value(f, "coal_weaponsmith", sett->coal_weaponsmith);

		save_text_write_value(f, "wheat_pigfarm", sett->wheat_pigfarm);
		save_text_write_value(f, "wheat_mill", sett->wheat_mill);

		save_text_write_value(f, "castle_score", sett->castle_score);

		/* TODO */

		fprintf(f, "\n");
	}

	return 0;
}

static int
save_text_flag_state(FILE *f)
{
	for (int i = 1; i < globals.max_ever_flag_index; i++) {
		if (FLAG_ALLOCATED(i)) {
			flag_t *flag = game_get_flag(i);

			fprintf(f, "[flag %i]\n", i);

			save_text_write_map_pos(f, "pos", flag->pos);
			save_text_write_value(f, "search_num", flag->search_num);
			save_text_write_value(f, "search_dir", flag->search_dir);
			save_text_write_value(f, "path_con", flag->path_con);
			save_text_write_value(f, "endpoints", flag->endpoint);
			save_text_write_value(f, "transporter", flag->transporter);

			save_text_write_array(f, "length", flag->length, 6);

			save_text_write_array(f, "resources.type", flag->res_waiting, 8);
			save_text_write_array(f, "resources.dest", flag->res_dest, 8);

			int indices[6];
			for (dir_t d = DIR_RIGHT; d <= DIR_UP; d++) {
				if (BIT_TEST(flag->endpoint, d)) {
					indices[d] = FLAG_INDEX(flag->other_endpoint.f[d]);
				} else {
					indices[d] = 0;
				}
			}

			if (BIT_TEST(flag->endpoint, 6)) {
				indices[DIR_UP_LEFT] = BUILDING_INDEX(flag->other_endpoint.b[DIR_UP_LEFT]);
			}

			save_text_write_array(f, "other_endpoint", indices, 6);

			save_text_write_value(f, "bld_flags", flag->bld_flags);
			save_text_write_value(f, "stock1_prio", flag->stock1_prio);
			save_text_write_value(f, "bld2_flags", flag->bld2_flags);
			save_text_write_value(f, "stock2_prio", flag->stock2_prio);

			fprintf(f, "\n");
		}
	}

	return 0;
}

static int
save_text_building_state(FILE *f)
{
	for (int i = 1; i < globals.max_ever_building_index; i++) {
		if (BUILDING_ALLOCATED(i)) {
			building_t *building = game_get_building(i);

			fprintf(f, "[building %i]\n", i);

			save_text_write_map_pos(f, "pos", building->pos);
			save_text_write_value(f, "bld", building->bld);
			save_text_write_value(f, "serf", building->serf);
			save_text_write_value(f, "flag_index", building->flg_index);
			save_text_write_value(f, "stock1", building->stock1);
			save_text_write_value(f, "stock2", building->stock2);
			save_text_write_value(f, "serf_index", building->serf_index);
			save_text_write_value(f, "progress", building->progress);

			if (!BUILDING_IS_BURNING(building) &&
			    (BUILDING_IS_DONE(building) ||
			     BUILDING_TYPE(building) == BUILDING_CASTLE)) {
				if (BUILDING_TYPE(building) == BUILDING_STOCK ||
				    BUILDING_TYPE(building) == BUILDING_CASTLE) {
					save_text_write_value(f, "inventory", INVENTORY_INDEX(building->u.inventory));
				} else {
					save_text_write_value(f, "flag", FLAG_INDEX(building->u.flag));
				}
			} else {
				save_text_write_value(f, "level", building->u.s.level);
				save_text_write_value(f, "planks_needed", building->u.s.planks_needed);
				save_text_write_value(f, "stone_needed", building->u.s.stone_needed);
			}

			fprintf(f, "\n");
		}
	}

	return 0;
}

static int
save_text_inventory_state(FILE *f)
{
	for (int i = 0; i < globals.max_ever_inventory_index; i++) {
		if (INVENTORY_ALLOCATED(i)) {
			inventory_t *inventory = game_get_inventory(i);

			fprintf(f, "[inventory %i]\n", i);

			save_text_write_value(f, "player", inventory->player_num);
			save_text_write_value(f, "res_dir", inventory->res_dir);
			save_text_write_value(f, "flag", inventory->flg_index);
			save_text_write_value(f, "building", inventory->bld_index);

			save_text_write_array(f, "queue.type", inventory->out_queue, 2);
			save_text_write_array(f, "queue.dest", inventory->out_dest, 2);

			save_text_write_value(f, "spawn_priority", inventory->spawn_priority);

			save_text_write_array(f, "resources", inventory->resources, 26);
			save_text_write_array(f, "serfs", inventory->serfs, 27);

			fprintf(f, "\n");
		}
	}

	return 0;
}

static int
save_text_serf_state(FILE *f)
{
	for (int i = 1; i < globals.max_ever_serf_index; i++) {
		if (!SERF_ALLOCATED(i)) continue;

		serf_t *serf = game_get_serf(i);

		fprintf(f, "[serf %i]\n", i);

		save_text_write_value(f, "type", serf->type);
		save_text_write_value(f, "animation", serf->animation);
		save_text_write_value(f, "counter", serf->counter);
		save_text_write_map_pos(f, "pos", serf->pos);
		save_text_write_value(f, "anim", serf->anim);
		save_text_write_value(f, "state", serf->state);

		switch (serf->state) {
		case SERF_STATE_IDLE_IN_STOCK:
			save_text_write_value(f, "state.inventory", serf->s.idle_in_stock.inv_index);
			break;

		case SERF_STATE_WALKING:
		case SERF_STATE_TRANSPORTING:
		case SERF_STATE_DELIVERING:
			save_text_write_value(f, "state.res", serf->s.walking.res);
			save_text_write_value(f, "state.dest", serf->s.walking.dest);
			save_text_write_value(f, "state.dir", serf->s.walking.dir);
			save_text_write_value(f, "state.wait_counter", serf->s.walking.wait_counter);
			break;

		case SERF_STATE_ENTERING_BUILDING:
			save_text_write_value(f, "state.field_B", serf->s.entering_building.field_B);
			save_text_write_value(f, "state.slope_len", serf->s.entering_building.slope_len);
			break;

		case SERF_STATE_LEAVING_BUILDING:
		case SERF_STATE_READY_TO_LEAVE:
			save_text_write_value(f, "state.field_B", serf->s.leaving_building.field_B);
			save_text_write_value(f, "state.dest", serf->s.leaving_building.dest);
			save_text_write_value(f, "state.dest2", serf->s.leaving_building.dest2);
			save_text_write_value(f, "state.dir", serf->s.leaving_building.dir);
			save_text_write_value(f, "state.next_state", serf->s.leaving_building.next_state);
			break;

		case SERF_STATE_READY_TO_ENTER:
			save_text_write_value(f, "state.field_B", serf->s.ready_to_enter.field_B);
			break;

		case SERF_STATE_DIGGING:
			save_text_write_value(f, "state.h_index", serf->s.digging.h_index);
			save_text_write_value(f, "state.target_h", serf->s.digging.target_h);
			save_text_write_value(f, "state.dig_pos", serf->s.digging.dig_pos);
			save_text_write_value(f, "state.substate", serf->s.digging.substate);
			break;

		case SERF_STATE_BUILDING:
			save_text_write_value(f, "state.mode", serf->s.building.mode);
			save_text_write_value(f, "state.bld_index", serf->s.building.bld_index);
			save_text_write_value(f, "state.material_step", serf->s.building.material_step);
			save_text_write_value(f, "state.counter", serf->s.building.counter);
			break;

		case SERF_STATE_BUILDING_CASTLE:
			save_text_write_value(f, "state.inv_index", serf->s.building_castle.inv_index);
			break;

		case SERF_STATE_MOVE_RESOURCE_OUT:
		case SERF_STATE_DROP_RESOURCE_OUT:
			save_text_write_value(f, "state.res", serf->s.move_resource_out.res);
			save_text_write_value(f, "state.res_dest", serf->s.move_resource_out.res_dest);
			save_text_write_value(f, "state.next_state", serf->s.move_resource_out.next_state);
			break;

		case SERF_STATE_READY_TO_LEAVE_INVENTORY:
			save_text_write_value(f, "state.mode", serf->s.ready_to_leave_inventory.mode);
			save_text_write_value(f, "state.dest", serf->s.ready_to_leave_inventory.dest);
			save_text_write_value(f, "state.inv_index", serf->s.ready_to_leave_inventory.inv_index);
			break;

		case SERF_STATE_FREE_WALKING:
		case SERF_STATE_LOGGING:
		case SERF_STATE_PLANTING:
		case SERF_STATE_STONECUTTING:
		case SERF_STATE_STONECUTTER_FREE_WALKING:
		case SERF_STATE_FISHING:
		case SERF_STATE_FARMING:
		case SERF_STATE_SAMPLING_GEO_SPOT:
			save_text_write_value(f, "state.dist1", serf->s.free_walking.dist1);
			save_text_write_value(f, "state.dist2", serf->s.free_walking.dist2);
			save_text_write_value(f, "state.neg_dist", serf->s.free_walking.neg_dist1);
			save_text_write_value(f, "state.neg_dist2", serf->s.free_walking.neg_dist2);
			save_text_write_value(f, "state.flags", serf->s.free_walking.flags);
			break;

		case SERF_STATE_SAWING:
			save_text_write_value(f, "state.mode", serf->s.sawing.mode);
			break;

		case SERF_STATE_LOST:
			save_text_write_value(f, "state.field_B", serf->s.lost.field_B);
			break;

		case SERF_STATE_MINING:
			save_text_write_value(f, "state.substate", serf->s.mining.substate);
			save_text_write_value(f, "state.res", serf->s.mining.res);
			save_text_write_value(f, "state.deposit", serf->s.mining.deposit);
			break;

		case SERF_STATE_SMELTING:
			save_text_write_value(f, "state.mode", serf->s.smelting.mode);
			save_text_write_value(f, "state.counter", serf->s.smelting.counter);
			save_text_write_value(f, "state.type", serf->s.smelting.type);
			break;

		case SERF_STATE_MILLING:
			save_text_write_value(f, "state.mode", serf->s.milling.mode);
			break;

		case SERF_STATE_BAKING:
			save_text_write_value(f, "state.mode", serf->s.baking.mode);
			break;

		case SERF_STATE_PIGFARMING:
			save_text_write_value(f, "state.mode", serf->s.pigfarming.mode);
			break;

		case SERF_STATE_BUTCHERING:
			save_text_write_value(f, "state.mode", serf->s.butchering.mode);
			break;

		case SERF_STATE_MAKING_WEAPON:
			save_text_write_value(f, "state.mode", serf->s.making_weapon.mode);
			break;

		case SERF_STATE_MAKING_TOOL:
			save_text_write_value(f, "state.mode", serf->s.making_tool.mode);
			break;

		case SERF_STATE_BUILDING_BOAT:
			save_text_write_value(f, "state.mode", serf->s.building_boat.mode);
			break;

		case SERF_STATE_IDLE_ON_PATH:
		case SERF_STATE_WAIT_IDLE_ON_PATH:
		case SERF_STATE_WAKE_AT_FLAG:
		case SERF_STATE_WAKE_ON_PATH:
			save_text_write_value(f, "state.rev_dir", serf->s.idle_on_path.rev_dir);
			save_text_write_value(f, "state.flag", FLAG_INDEX(serf->s.idle_on_path.flag));
			save_text_write_value(f, "state.field_E", serf->s.idle_on_path.field_E);
			break;

		case SERF_STATE_DEFENDING_HUT:
		case SERF_STATE_DEFENDING_TOWER:
		case SERF_STATE_DEFENDING_FORTRESS:
		case SERF_STATE_DEFENDING_CASTLE:
			save_text_write_value(f, "state.next_knight", serf->s.defending.next_knight);
			break;

		default: break;
		}

		fprintf(f, "\n");
	}

	return 0;
}

static int
save_text_map_state(FILE *f)
{
	for (int y = 0; y < globals.map.rows; y++) {
		for (int x = 0; x < globals.map.cols; x++) {
			map_pos_t pos = MAP_POS(x, y);

			fprintf(f, "[map %i %i]\n", x, y);

			save_text_write_value(f, "deep_water", MAP_DEEP_WATER(pos));
			save_text_write_value(f, "paths", MAP_PATHS(pos));
			save_text_write_value(f, "has_owner", MAP_HAS_OWNER(pos));
			save_text_write_value(f, "owner", MAP_OWNER(pos));
			save_text_write_value(f, "height", MAP_HEIGHT(pos));
			save_text_write_value(f, "type.up", MAP_TYPE_UP(pos));
			save_text_write_value(f, "type.down", MAP_TYPE_DOWN(pos));
			save_text_write_value(f, "object", MAP_OBJ(pos));
			save_text_write_value(f, "water", MAP_WATER(pos));
			save_text_write_value(f, "serf_index", MAP_SERF_INDEX(pos));

			if (MAP_OBJ(pos) >= MAP_OBJ_FLAG &&
			    MAP_OBJ(pos) <= MAP_OBJ_CASTLE) {
				save_text_write_value(f, "object_index", MAP_OBJ_INDEX(pos));
			} else if (MAP_DEEP_WATER(pos)) {
				save_text_write_value(f, "idle_serf", MAP_IDLE_SERF(pos));
				save_text_write_value(f, "player", MAP_PLAYER(pos));
				save_text_write_value(f, "fish", MAP_RES_FISH(pos));
			} else {
				save_text_write_value(f, "idle_serf", MAP_IDLE_SERF(pos));
				save_text_write_value(f, "player", MAP_PLAYER(pos));
				save_text_write_value(f, "resource.type", MAP_RES_TYPE(pos));
				save_text_write_value(f, "resource.amount", MAP_RES_AMOUNT(pos));
			}

			fprintf(f, "\n");
		}
	}

	return 0;
}

int
save_text_state(FILE *f)
{
	int r;

	r = save_text_globals_state(f);
	if (r < 0) return -1;

	r = save_text_player_state(f);
	if (r < 0) return -1;

	r = save_text_flag_state(f);
	if (r < 0) return -1;

	r = save_text_building_state(f);
	if (r < 0) return -1;

	r = save_text_inventory_state(f);
	if (r < 0) return -1;

	r = save_text_serf_state(f);
	if (r < 0) return -1;

	r = save_text_map_state(f);
	if (r < 0) return -1;

	return 0;
}


static char *
trim_whitespace(char *str, size_t *len)
{
	/* Left */
	while (isspace(str[0])) {
		str += 1;
		*len -= 1;
	}

	/* Right */
	if (*len > 0) {
		char *last = str + *len - 1;
		while (isspace(last[0])) {
			last[0] = '\0';
			last -= 1;
			*len -= 1;
		}
	}

	return str;
}

/* Read line from f, realloc'ing the buffer
   if necessary. Return bytes read (0 on EOF). */
static size_t
load_text_readline(char **buffer, size_t *len, FILE *f)
{
	size_t i = 0;

	while (1) {
		if (i >= *len-1) {
			/* Double buffer size, always keep enough
			   space for input character plus zero-byte. */
			*len = 2*(*len);
			*buffer = realloc(*buffer, *len);
		}

		/* Read character */
		int c = fgetc(f);
		if (c == EOF) {
			(*buffer)[i] = '\0';
			break;
		}

		(*buffer)[i++] = c;

		if (c == '\n') {
			(*buffer)[i] = '\0';
			break;
		}
	}

	return i;
}


typedef struct {
	list_elm_t elm;
	char *name;
	char *param;
	list_t settings;
} section_t;

typedef struct {
	list_elm_t elm;
	char *key;
	char *value;
} setting_t;


static int
load_text_parse(FILE *f, list_t *sections)
{
	size_t buffer_len = 256;
	char *buffer = malloc(buffer_len*sizeof(char));

	section_t *section = NULL;

	while (1) {
		/* Read line from file */
		char *line = NULL;
		size_t line_len = 0;
		size_t len = load_text_readline(&buffer, &buffer_len, f);
		if (len == 0) {
			if (ferror(f)) return -1;
			break;
		}

		line_len = len;
		line = buffer;

		/* Skip leading whitespace */
		line = trim_whitespace(line, &line_len);

		/* Skip blank lines */
		if (line_len == 0) continue;

		if (line[0] == '[')  {
			/* Section header */
			char *header = &line[1];
			char *header_end = strchr(line, ']');
			if (header_end == NULL) {
				LOGW("savegame", "Malformed section header: `%s'.",
				     header);
				continue;
			}

			size_t header_len = header_end - header;
			header_end[0] = '\0';
			header = trim_whitespace(header, &header_len);

			/* Extract parameter */
			char *param = header;
			while (param[0] != '\0' &&
			       !isspace(param[0])) {
				param += 1;
			}

			if (isspace(param[0])) {
				param[0] = '\0';
				param += 1;
				header_len = param - header;
				while (isspace(param[0])) param += 1;
			}

			/* Create section */
			section = malloc(sizeof(section_t));
			if (section == NULL) abort();

			section->name = strdup(header);
			section->param = strdup(param);
			list_init(&section->settings);
			list_append(sections, (list_elm_t *)section);
		} else if (section != NULL) {
			/* Setting line */
			char *value = strchr(line, '=');
			if (value == NULL) {
				LOGW("savegame", "Malformed setting line: `%s'.", line);
				continue;
			}

			/* Key */
			char *key = line;
			size_t key_len = value - key;
			key = trim_whitespace(key, &key_len);

			/* Value */
			value[0] = '\0';
			value += 1;
			while (isspace(value[0])) value += 1;

			setting_t *setting = malloc(sizeof(setting_t));
			if (setting == NULL) abort();

			setting->key = strdup(key);
			setting->value = strdup(value);
			list_append(&section->settings, (list_elm_t *)setting);
		}
	}

	free(buffer);

	return 0;
}

static void
load_text_free_sections(list_t *sections)
{
	while (!list_is_empty(sections)) {
		section_t *section =
			(section_t *)list_remove_head(sections);
		free(section->name);
		free(section->param);

		while (!list_is_empty(&section->settings)) {
			setting_t *setting =
				(setting_t *)list_remove_head(&section->settings);
			free(setting->key);
			free(setting->value);
			free(setting);
		}

		free(section);
	}
}

static char *
load_text_get_setting(const section_t *section, const char *key)
{
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, key)) {
			return s->value;
		}
	}

	return NULL;
}

static map_pos_t
parse_map_pos(char *str)
{
	char *s = strchr(str, ',');
	if (s == NULL) return 0;

	s[0] = '\0';
	s += 1;

	return MAP_POS(atoi(str), atoi(s));
}

static char *
parse_array_value(char **str)
{
	char *value = *str;
	char *next = strchr(*str, ',');
	if (next == NULL) {
		*str = NULL;
		return value;
	}

	*str = next+1;
	return value;
}

static int
load_text_global_state(list_t *sections)
{
	const char *value;

	/* Find the globals section */
	section_t *section = NULL;
	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "globals")) {
			section = s;
			break;
		}
	}

	if (section == NULL) return -1;

	/* Load essential values for calculating map positions
	   so that map positions can be loaded properly. */
	value = load_text_get_setting(section, "map.col_size");
	if (value == NULL) return -1;
	globals.map.col_size = atoi(value);

	value = load_text_get_setting(section, "map.row_size");
	if (value == NULL) return -1;
	globals.map.row_size = atoi(value);

	/* Initialize remaining map dimensions. */
	globals.map.cols = 1 << globals.map.col_size;
	globals.map.rows = 1 << globals.map.row_size;
	map_init_dimensions(&globals.map);

	/* Load the remaining global state. */
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "version")) {
			LOGV("savegame", "Loading save game from version %s.", s->value);
		} else if (!strcmp(s->key, "map.col_size") ||
			   !strcmp(s->key, "map.row_size")) {
			/* Already loaded above. */
		} else if (!strcmp(s->key, "split")) {
			globals.split = atoi(s->value);
		} else if (!strcmp(s->key, "update_map_initial_pos")) {
			globals.update_map_initial_pos = parse_map_pos(s->value);
		} else if (!strcmp(s->key, "cfg.left")) {
			globals.cfg_left = atoi(s->value);
		} else if (!strcmp(s->key, "cfg.right")) {
			globals.cfg_right = atoi(s->value);
		} else if (!strcmp(s->key, "game_type")) {
			globals.game_type = atoi(s->value);
		} else if (!strcmp(s->key, "game_tick")) {
			globals.game_tick = atoi(s->value);
		} else if (!strcmp(s->key, "game_stats_counter")) {
			globals.game_stats_counter = atoi(s->value);
		} else if (!strcmp(s->key, "history_counter")) {
			globals.history_counter = atoi(s->value);
		} else if (!strcmp(s->key, "rnd")) {
			char *array = s->value;
			for (int i = 0; i < 3 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				globals.rnd.state[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "max_ever_flag_index")) {
			globals.max_ever_flag_index = atoi(s->value);
		} else if (!strcmp(s->key, "max_ever_building_index")) {
			globals.max_ever_building_index = atoi(s->value);
		} else if (!strcmp(s->key, "max_ever_serf_index")) {
			globals.max_ever_serf_index = atoi(s->value);
		} else if (!strcmp(s->key, "next_index")) {
			globals.next_index = atoi(s->value);
		} else if (!strcmp(s->key, "flag_search_counter")) {
			globals.flag_search_counter = atoi(s->value);
		} else if (!strcmp(s->key, "update_map_last_anim")) {
			globals.update_map_last_anim = atoi(s->value);
		} else if (!strcmp(s->key, "update_map_counter")) {
			globals.update_map_counter = atoi(s->value);
		} else if (!strcmp(s->key, "player_history_index")) {
			char *array = s->value;
			for (int i = 0; i < 4 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				globals.player_history_index[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "player_history_counter")) {
			char *array = s->value;
			for (int i = 0; i < 3 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				globals.player_history_counter[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "resource_history_index")) {
			globals.resource_history_index = atoi(s->value);
		} else if (!strcmp(s->key, "map.regions")) {
			globals.map_regions = atoi(s->value);
		} else if (!strcmp(s->key, "max_ever_inventory_index")) {
			globals.max_ever_inventory_index = atoi(s->value);
		} else if (!strcmp(s->key, "map.max_serfs_left")) {
			globals.map_max_serfs_left = atoi(s->value);
		} else if (!strcmp(s->key, "max_next_index")) {
			globals.max_next_index = atoi(s->value);
		} else if (!strcmp(s->key, "map.field_4A")) {
			globals.map_field_4A = atoi(s->value);
		} else if (!strcmp(s->key, "map.gold_deposit")) {
			globals.map_gold_deposit = atoi(s->value);
		} else if (!strcmp(s->key, "update_map_16_loop")) {
			globals.update_map_16_loop = atoi(s->value);
		} else if (!strcmp(s->key, "map.size")) {
			globals.map_size = atoi(s->value);
		} else if (!strcmp(s->key, "map.field_52")) {
			globals.map_field_52 = atoi(s->value);
		} else if (!strcmp(s->key, "map.62_5_times_regions")) {
			globals.map_62_5_times_regions = atoi(s->value);
		} else if (!strcmp(s->key, "map.gold_morale_factor")) {
			globals.map_gold_morale_factor = atoi(s->value);
		} else if (!strcmp(s->key, "winning_player")) {
			globals.winning_player = atoi(s->value);
		} else if (!strcmp(s->key, "player_score_leader")) {
			globals.player_score_leader = atoi(s->value);
		} else {
			LOGD("savegame", "Unhandled global setting: `%s'.", s->key);
		}
	}

	return 0;
}

static int
load_text_player_section(section_t *section)
{
	/* Parse player number. */
	int n = atoi(section->param);
	player_sett_t *sett = globals.player_sett[n];

	/* Load the player state. */
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "flags")) {
			sett->flags = atoi(s->value);
		} else if (!strcmp(s->key, "build")) {
			sett->build = atoi(s->value);
		} else if (!strcmp(s->key, "tool_prio")) {
			char *array = s->value;
			for (int i = 0; i < 9 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->tool_prio[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "resource_count")) {
			char *array = s->value;
			for (int i = 0; i < 26 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->resource_count[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "flag_prio")) {
			char *array = s->value;
			for (int i = 0; i < 26 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->flag_prio[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "serf_count")) {
			char *array = s->value;
			for (int i = 0; i < 27 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->serf_count[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "knight_occupation")) {
			char *array = s->value;
			for (int i = 0; i < 4 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->knight_occupation[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "completed_building_count")) {
			char *array = s->value;
			for (int i = 0; i < 23 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->completed_building_count[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "incomplete_building_count")) {
			char *array = s->value;
			for (int i = 0; i < 23 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->incomplete_building_count[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "inventory_prio")) {
			char *array = s->value;
			for (int i = 0; i < 26 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->inventory_prio[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "attacking_buildings")) {
			char *array = s->value;
			for (int i = 0; i < 64 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->attacking_buildings[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "map_cursor.col")) {
			sett->map_cursor_col = atoi(s->value);
		} else if (!strcmp(s->key, "map_cursor.row")) {
			sett->map_cursor_row = atoi(s->value);
		} else if (!strcmp(s->key, "knights_to_spawn")) {
			sett->knights_to_spawn = atoi(s->value);
		} else if (!strcmp(s->key, "last_anim")) {
			sett->last_anim = atoi(s->value);
		} else if (!strcmp(s->key, "reproduction_counter")) {
			sett->reproduction_counter = atoi(s->value);
		} else if (!strcmp(s->key, "reproduction_reset")) {
			sett->reproduction_reset = atoi(s->value);
		} else if (!strcmp(s->key, "serf_to_knight_rate")) {
			sett->serf_to_knight_rate = atoi(s->value);
		} else if (!strcmp(s->key, "serf_to_knight_counter")) {
			sett->serf_to_knight_counter = atoi(s->value);
		} else if (!strcmp(s->key, "attacking_building_count")) {
			sett->attacking_building_count = atoi(s->value);
		} else if (!strcmp(s->key, "attacking_knights")) {
			char *array = s->value;
			for (int i = 0; i < 4 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				sett->attacking_knights[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "total_attacking_knights")) {
			sett->total_attacking_knights = atoi(s->value);
		} else if (!strcmp(s->key, "building_attacked")) {
			sett->building_attacked = atoi(s->value);
		} else if (!strcmp(s->key, "knights_attacking")) {
			sett->knights_attacking = atoi(s->value);
		} else if (!strcmp(s->key, "food_stonemine")) {
			sett->food_stonemine = atoi(s->value);
		} else if (!strcmp(s->key, "food_coalmine")) {
			sett->food_coalmine = atoi(s->value);
		} else if (!strcmp(s->key, "food_ironmine")) {
			sett->food_ironmine = atoi(s->value);
		} else if (!strcmp(s->key, "food_goldmine")) {
			sett->food_goldmine = atoi(s->value);
		} else if (!strcmp(s->key, "planks_construction")) {
			sett->planks_construction = atoi(s->value);
		} else if (!strcmp(s->key, "planks_boatbuilder")) {
			sett->planks_boatbuilder = atoi(s->value);
		} else if (!strcmp(s->key, "planks_toolmaker")) {
			sett->planks_toolmaker = atoi(s->value);
		} else if (!strcmp(s->key, "steel_toolmaker")) {
			sett->steel_toolmaker = atoi(s->value);
		} else if (!strcmp(s->key, "steel_weaponsmith")) {
			sett->steel_weaponsmith = atoi(s->value);
		} else if (!strcmp(s->key, "coal_steelsmelter")) {
			sett->coal_steelsmelter = atoi(s->value);
		} else if (!strcmp(s->key, "coal_goldsmelter")) {
			sett->coal_goldsmelter = atoi(s->value);
		} else if (!strcmp(s->key, "coal_weaponsmith")) {
			sett->coal_weaponsmith = atoi(s->value);
		} else if (!strcmp(s->key, "wheat_pigfarm")) {
			sett->wheat_pigfarm = atoi(s->value);
		} else if (!strcmp(s->key, "wheat_mill")) {
			sett->wheat_mill = atoi(s->value);
		} else if (!strcmp(s->key, "castle_score")) {
			sett->castle_score = atoi(s->value);
		} else {
			LOGD("savegame", "Unhandled player setting: `%s'.", s->key);
		}
	}

	return 0;
}

static int
load_text_player_state(list_t *sections)
{
	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "player")) {
			int r = load_text_player_section(s);
			if (r < 0) return -1;
		}
	}

	return 0;
}

static int
load_text_flag_section(section_t *section)
{
	/* Parse flag number. */
	int n = atoi(section->param);
	if (n >= globals.max_flg_cnt) return -1;

	flag_t *flag = &globals.flgs[n];
	globals.flg_bitmap[n/8] |= BIT(7-(n&7));

	/* Load the flag state. */
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "pos")) {
			flag->pos = parse_map_pos(s->value);
		} else if (!strcmp(s->key, "search_num")) {
			flag->search_num = atoi(s->value);
		} else if (!strcmp(s->key, "search_dir")) {
			flag->search_dir = atoi(s->value);
		} else if (!strcmp(s->key, "path_con")) {
			flag->path_con = atoi(s->value);
		} else if (!strcmp(s->key, "endpoints")) {
			flag->endpoint = atoi(s->value);
		} else if (!strcmp(s->key, "transporter")) {
			flag->transporter = atoi(s->value);
		} else if (!strcmp(s->key, "length")) {
			char *array = s->value;
			for (int i = 0; i < 6 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				flag->length[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "resources.type")) {
			char *array = s->value;
			for (int i = 0; i < 8 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				flag->res_waiting[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "resources.dest")) {
			char *array = s->value;
			for (int i = 0; i < 8 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				flag->res_dest[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "other_endpoint")) {
			/* TODO make sure these pointers are valid. Maybe postpone this
			   linking until all flags are loaded. */
			char *array = s->value;
			for (int i = 0; i < 6 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				flag->other_endpoint.f[i] = &globals.flgs[atoi(v)];
			}
		} else if (!strcmp(s->key, "bld_flags")) {
			flag->bld_flags = atoi(s->value);
		} else if (!strcmp(s->key, "stock1_prio")) {
			flag->stock1_prio = atoi(s->value);
		} else if (!strcmp(s->key, "bld2_flags")) {
			flag->bld2_flags = atoi(s->value);
		} else if (!strcmp(s->key, "stock2_prio")) {
			flag->stock2_prio = atoi(s->value);
		} else {
			LOGD("savegame", "Unhandled flag setting: `%s'.", s->key);
		}
	}

	/* Fix link if connected to building*/
	if (BIT_TEST(flag->endpoint, 6)) {
		char *array = load_text_get_setting(section, "other_endpoint");
		char *v = NULL;
		for (dir_t d = DIR_RIGHT; d <= DIR_UP_LEFT && array != NULL; d++) {
			char *r = parse_array_value(&array);
			if (d == DIR_UP_LEFT) v = r;
		}

		if (v == NULL) return -1;

		flag->other_endpoint.b[DIR_UP_LEFT] = &globals.buildings[atoi(v)];
	}

	return 0;
}

static int
load_text_flag_state(list_t *sections)
{
	/* Clear flag allocation bitmap */
	memset(globals.flg_bitmap, 0, ((globals.max_flg_cnt-1) / 8) + 1);

	/* Create NULL-flag (index 0 is undefined) */
	game_alloc_flag(NULL, NULL);

	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "flag")) {
			int r = load_text_flag_section(s);
			if (r < 0) return -1;
		}
	}

	return 0;
}

static int
load_text_building_section(section_t *section)
{
	/* Parse building number. */
	int n = atoi(section->param);
	if (n >= globals.max_building_cnt) return -1;

	building_t *building = &globals.buildings[n];
	globals.buildings_bitmap[n/8] |= BIT(7-(n&7));

	/* Load the building state. */
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "pos")) {
			building->pos = parse_map_pos(s->value);
		} else if (!strcmp(s->key, "bld")) {
			building->bld = atoi(s->value);
		} else if (!strcmp(s->key, "serf")) {
			building->serf = atoi(s->value);
		} else if (!strcmp(s->key, "flag_index")) {
			building->flg_index = atoi(s->value);
		} else if (!strcmp(s->key, "stock1")) {
			building->stock1 = atoi(s->value);
		} else if (!strcmp(s->key, "stock2")) {
			building->stock2 = atoi(s->value);
		} else if (!strcmp(s->key, "serf_index")) {
			building->serf_index = atoi(s->value);
		} else if (!strcmp(s->key, "progress")) {
			building->progress = atoi(s->value);
		} else if (!strcmp(s->key, "inventory") ||
			   !strcmp(s->key, "flag") ||
			   !strcmp(s->key, "level") ||
			   !strcmp(s->key, "planks_needed") ||
			   !strcmp(s->key, "stone_needed")) {
			/* Handled later */
		} else {
			LOGD("savegame", "Unhandled building setting: `%s'.", s->key);
		}
	}

	/* Load various values that depend on the building type. */
	/* TODO Check validity of pointers when loading. */
	if (!BUILDING_IS_BURNING(building) &&
	    (BUILDING_IS_DONE(building) ||
	     BUILDING_TYPE(building) == BUILDING_CASTLE)) {
		if (BUILDING_TYPE(building) == BUILDING_STOCK ||
		    BUILDING_TYPE(building) == BUILDING_CASTLE) {
			char *value = load_text_get_setting(section, "inventory");
			if (value == NULL) return -1;
			building->u.inventory = &globals.inventories[atoi(value)];
		} else {
			char *value = load_text_get_setting(section, "flag");
			if (value == NULL) return -1;
			building->u.flag = &globals.flgs[atoi(value)];
		}
	} else {
		list_foreach(&section->settings, elm) {
			setting_t *s = (setting_t *)elm;
			if (!strcmp(s->key, "level")) {
				building->u.s.level = atoi(s->value);
			} else if (!strcmp(s->key, "planks_needed")) {
				building->u.s.planks_needed = atoi(s->value);
			} else if (!strcmp(s->key, "stone_needed")) {
				building->u.s.stone_needed = atoi(s->value);
			}
		}
	}

	return 0;
}

static int
load_text_building_state(list_t *sections)
{
	/* Clear building allocation bitmap */
	memset(globals.buildings_bitmap, 0, ((globals.max_building_cnt-1) / 8) + 1);

	/* Create NULL-building (index 0 is undefined) */
	building_t *building;
	game_alloc_building(&building, NULL);
	building->bld = 0;

	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "building")) {
			int r = load_text_building_section(s);
			if (r < 0) return -1;
		}
	}

	return 0;
}

static int
load_text_inventory_section(section_t *section)
{
	/* Parse building number. */
	int n = atoi(section->param);
	if (n >= globals.max_inventory_cnt) return -1;

	inventory_t *inventory = &globals.inventories[n];
	globals.inventories_bitmap[n/8] |= BIT(7-(n&7));

	/* Load the inventory state. */
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "player")) {
			inventory->player_num = atoi(s->value);
		} else if (!strcmp(s->key, "res_dir")) {
			inventory->res_dir = atoi(s->value);
		} else if (!strcmp(s->key, "flag")) {
			inventory->flg_index = atoi(s->value);
		} else if (!strcmp(s->key, "building")) {
			inventory->bld_index = atoi(s->value);
		} else if (!strcmp(s->key, "queue.type")) {
			char *array = s->value;
			for (int i = 0; i < 2 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				inventory->out_queue[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "queue.dest")) {
			char *array = s->value;
			for (int i = 0; i < 2 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				inventory->out_dest[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "spawn_priority")) {
			inventory->spawn_priority = atoi(s->value);
		} else if (!strcmp(s->key, "resources")) {
			char *array = s->value;
			for (int i = 0; i < 26 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				inventory->resources[i] = atoi(v);
			}
		} else if (!strcmp(s->key, "serfs")) {
			char *array = s->value;
			for (int i = 0; i < 27 && array != NULL; i++) {
				char *v = parse_array_value(&array);
				inventory->serfs[i] = atoi(v);
			}
		} else {
			LOGD("savegame", "Unhandled inventory setting: `%s'.", s->key);
		}
	}

	return 0;
}

static int
load_text_inventory_state(list_t *sections)
{
	/* Clear inventory allocation bitmap */
	memset(globals.inventories_bitmap, 0, ((globals.max_inventory_cnt-1) / 8) + 1);

	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "inventory")) {
			int r = load_text_inventory_section(s);
			if (r < 0) return -1;
		}
	}

	return 0;
}

static int
load_text_serf_section(section_t *section)
{
	/* Parse serf number. */
	int n = atoi(section->param);
	if (n >= globals.max_serf_cnt) return -1;

	serf_t *serf = &globals.serfs[n];
	globals.serfs_bitmap[n/8] |= BIT(7-(n&7));

	/* Load the serf state. */
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "type")) {
			serf->type = atoi(s->value);
		} else if (!strcmp(s->key, "animation")) {
			serf->animation = atoi(s->value);
		} else if (!strcmp(s->key, "counter")) {
			serf->counter = atoi(s->value);
		} else if (!strcmp(s->key, "pos")) {
			serf->pos = parse_map_pos(s->value);
		} else if (!strcmp(s->key, "anim")) {
			serf->anim = atoi(s->value);
		} else if (!strcmp(s->key, "state")) {
			serf->state = atoi(s->value);
		} else if (!strncmp(s->key, "state.", strlen("state."))) {
			/* Handled later */
		} else {
			LOGD("savegame", "Unhandled serf setting: `%s'.", s->key);
		}
	}

	/* Load state variables */
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		switch (serf->state) {
		case SERF_STATE_IDLE_IN_STOCK:
			if (!strcmp(s->key, "state.inventory")) {
				serf->s.idle_in_stock.inv_index = atoi(s->value);
			}
			break;

		case SERF_STATE_WALKING:
		case SERF_STATE_TRANSPORTING:
		case SERF_STATE_DELIVERING:
			if (!strcmp(s->key, "state.res")) {
				serf->s.walking.res = atoi(s->value);
			} else if (!strcmp(s->key, "state.dest")) {
				serf->s.walking.dest = atoi(s->value);
			} else if (!strcmp(s->key, "state.dir")) {
				serf->s.walking.dir = atoi(s->value);
			} else if (!strcmp(s->key, "state.wait_counter")) {
				serf->s.walking.wait_counter = atoi(s->value);
			}
			break;

		case SERF_STATE_ENTERING_BUILDING:
			if (!strcmp(s->key, "state.field_B")) {
				serf->s.entering_building.field_B = atoi(s->value);
			} else if (!strcmp(s->key, "state.slope_len")) {
				serf->s.entering_building.slope_len = atoi(s->value);
			}
			break;

		case SERF_STATE_LEAVING_BUILDING:
		case SERF_STATE_READY_TO_LEAVE:
			if (!strcmp(s->key, "state.field_B")) {
				serf->s.leaving_building.field_B = atoi(s->value);
			} else if (!strcmp(s->key, "state.dest")) {
				serf->s.leaving_building.dest = atoi(s->value);
			} else if (!strcmp(s->key, "state.dest2")) {
				serf->s.leaving_building.dest2 = atoi(s->value);
			} else if (!strcmp(s->key, "state.dir")) {
				serf->s.leaving_building.dir = atoi(s->value);
			} else if (!strcmp(s->key, "state.next_state")) {
				serf->s.leaving_building.next_state = atoi(s->value);
			}
			break;

		case SERF_STATE_READY_TO_ENTER:
			if (!strcmp(s->key, "state.field_B")) {
				serf->s.ready_to_enter.field_B = atoi(s->value);
			}
			break;

		case SERF_STATE_DIGGING:
			if (!strcmp(s->key, "state.h_index")) {
				serf->s.digging.h_index = atoi(s->value);
			} else if (!strcmp(s->key, "state.target_h")) {
				serf->s.digging.target_h = atoi(s->value);
			} else if (!strcmp(s->key, "state.dig_pos")) {
				serf->s.digging.dig_pos = atoi(s->value);
			} else if (!strcmp(s->key, "state.substate")) {
				serf->s.digging.substate = atoi(s->value);
			}
			break;

		case SERF_STATE_BUILDING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.building.mode = atoi(s->value);
			} else if (!strcmp(s->key, "state.bld_index")) {
				serf->s.building.bld_index = atoi(s->value);
			} else if (!strcmp(s->key, "state.material_step")) {
				serf->s.building.material_step = atoi(s->value);
			} else if (!strcmp(s->key, "state.counter")) {
				serf->s.building.counter = atoi(s->value);
			}
			break;

		case SERF_STATE_BUILDING_CASTLE:
			if (!strcmp(s->key, "state.inv_index")) {
				serf->s.building_castle.inv_index = atoi(s->value);
			}
			break;

		case SERF_STATE_MOVE_RESOURCE_OUT:
		case SERF_STATE_DROP_RESOURCE_OUT:
			if (!strcmp(s->key, "state.res")) {
				serf->s.move_resource_out.res = atoi(s->value);
			} else if (!strcmp(s->key, "state.res_dest")) {
				serf->s.move_resource_out.res_dest = atoi(s->value);
			} else if (!strcmp(s->key, "state.next_state")) {
				serf->s.move_resource_out.next_state = atoi(s->value);
			}
			break;

		case SERF_STATE_READY_TO_LEAVE_INVENTORY:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.ready_to_leave_inventory.mode = atoi(s->value);
			} else if (!strcmp(s->key, "state.dest")) {
				serf->s.ready_to_leave_inventory.dest = atoi(s->value);
			} else if (!strcmp(s->key, "state.inv_index")) {
				serf->s.ready_to_leave_inventory.inv_index = atoi(s->value);
			}
			break;

		case SERF_STATE_FREE_WALKING:
		case SERF_STATE_LOGGING:
		case SERF_STATE_PLANTING:
		case SERF_STATE_STONECUTTING:
		case SERF_STATE_STONECUTTER_FREE_WALKING:
		case SERF_STATE_FISHING:
		case SERF_STATE_FARMING:
		case SERF_STATE_SAMPLING_GEO_SPOT:
			if (!strcmp(s->key, "state.dist1")) {
				serf->s.free_walking.dist1 = atoi(s->value);
			} else if (!strcmp(s->key, "state.dist2")) {
				serf->s.free_walking.dist2 = atoi(s->value);
			} else if (!strcmp(s->key, "state.neg_dist")) {
				serf->s.free_walking.neg_dist1 = atoi(s->value);
			} else if (!strcmp(s->key, "state.neg_dist2")) {
				serf->s.free_walking.neg_dist2 = atoi(s->value);
			} else if (!strcmp(s->key, "state.flags")) {
				serf->s.free_walking.flags = atoi(s->value);
			}
			break;

		case SERF_STATE_SAWING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.sawing.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_LOST:
			if (!strcmp(s->key, "state.field_B")) {
				serf->s.lost.field_B = atoi(s->value);
			}
			break;

		case SERF_STATE_MINING:
			if (!strcmp(s->key, "state.substate")) {
				serf->s.mining.substate = atoi(s->value);
			} else if (!strcmp(s->key, "state.res")) {
				serf->s.mining.res = atoi(s->value);
			} else if (!strcmp(s->key, "state.deposit")) {
				serf->s.mining.deposit = atoi(s->value);
			}
			break;

		case SERF_STATE_SMELTING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.smelting.mode = atoi(s->value);
			} else if (!strcmp(s->key, "state.counter")) {
				serf->s.smelting.counter = atoi(s->value);
			} else if (!strcmp(s->key, "state.type")) {
				serf->s.smelting.type = atoi(s->value);
			}
			break;

		case SERF_STATE_MILLING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.milling.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_BAKING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.baking.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_PIGFARMING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.pigfarming.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_BUTCHERING:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.butchering.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_MAKING_WEAPON:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.making_weapon.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_MAKING_TOOL:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.making_tool.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_BUILDING_BOAT:
			if (!strcmp(s->key, "state.mode")) {
				serf->s.building_boat.mode = atoi(s->value);
			}
			break;

		case SERF_STATE_IDLE_ON_PATH:
		case SERF_STATE_WAIT_IDLE_ON_PATH:
		case SERF_STATE_WAKE_AT_FLAG:
		case SERF_STATE_WAKE_ON_PATH:
			if (!strcmp(s->key, "state.rev_dir")) {
				serf->s.idle_on_path.rev_dir = atoi(s->value);
			} else if (!strcmp(s->key, "state.flag")) {
				serf->s.idle_on_path.flag = &globals.flgs[atoi(s->value)];
			} else if (!strcmp(s->key, "state.field_E")) {
				serf->s.idle_on_path.field_E = atoi(s->value);
			}
			break;

		case SERF_STATE_DEFENDING_HUT:
		case SERF_STATE_DEFENDING_TOWER:
		case SERF_STATE_DEFENDING_FORTRESS:
		case SERF_STATE_DEFENDING_CASTLE:
			if (!strcmp(s->key, "state.next_knight")) {
				serf->s.defending.next_knight = atoi(s->value);
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

static int
load_text_serf_state(list_t *sections)
{
	/* Clear serf allocation bitmap */
	memset(globals.serfs_bitmap, 0, ((globals.max_serf_cnt-1) / 8) + 1);

	/* Create NULL-serf */
	serf_t *serf;
	game_alloc_serf(&serf, NULL);
	serf->state = SERF_STATE_NULL;
	serf->type = 0;
	serf->animation = 0;
	serf->counter = 0;
	serf->pos = -1;

	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "serf")) {
			int r = load_text_serf_section(s);
			if (r < 0) return -1;
		}
	}

	return 0;
}

static int
load_text_map_section(section_t *section)
{
	char *param = section->param;

	/* Parse map position. */
	int col = atoi(param);
	if (col < 0 || col >= globals.map.cols) return -1;

	while (!isspace(*param) && *param != '\0') param += 1;
	while (isspace(*param) && *param != '\0') param += 1;
	if (*param == '\0') return -1;

	int row = atoi(param);
	if (row < 0 || row >= globals.map.rows) return -1;

	map_pos_t pos = MAP_POS(col,row);
	map_tile_t *tiles = globals.map.tiles;

	uint deep_water = 0;
	uint paths = 0;
	uint has_owner = 0;
	uint owner = 0;
	uint height = 0;
	uint type_up = 0;
	uint type_down = 0;
	map_obj_t obj = MAP_OBJ_NONE;
	uint water = 0;

	/* Load the map tile. */
	list_elm_t *elm;
	list_foreach(&section->settings, elm) {
		setting_t *s = (setting_t *)elm;
		if (!strcmp(s->key, "deep_water")) {
			deep_water = atoi(s->value);
		} else if (!strcmp(s->key, "paths")) {
			paths = atoi(s->value);
		} else if (!strcmp(s->key, "has_owner")) {
			has_owner = atoi(s->value);
		} else if (!strcmp(s->key, "owner")) {
			owner = atoi(s->value);
		} else if (!strcmp(s->key, "height")) {
			height = atoi(s->value);
		} else if (!strcmp(s->key, "type.up")) {
			type_up = atoi(s->value);
		} else if (!strcmp(s->key, "type.down")) {
			type_down = atoi(s->value);
		} else if (!strcmp(s->key, "object")) {
			obj = atoi(s->value);
		} else if (!strcmp(s->key, "water")) {
			water = atoi(s->value);
		} else if (!strcmp(s->key, "serf_index")) {
			tiles[pos].serf_index = atoi(s->value);
		} else if (!strcmp(s->key, "object_index") ||
			   !strcmp(s->key, "idle_serf") ||
			   !strcmp(s->key, "player") ||
			   !strcmp(s->key, "fish") ||
			   !strcmp(s->key, "resource.type") ||
			   !strcmp(s->key, "resource.amount")) {
			/* Handled later */
		} else {
			LOGD("savegame", "Unhandled map setting: `%s'.", s->key);
		}
	}

	tiles[pos].flags = ((deep_water & 1) << 6) | (paths & 0x3f);
	tiles[pos].height = ((has_owner & 1) << 7) |
		((owner & 3) << 5) | (height & 0x1f);
	tiles[pos].type = ((type_up & 0xf) << 4) | (type_down & 0xf);
	tiles[pos].obj = ((water & 1) << 7) | (obj & 0x7f);

	/* Set has_flag bit */
	if (MAP_OBJ(pos) == MAP_OBJ_FLAG) tiles[pos].flags |= BIT(7);

	if (MAP_OBJ(pos) >= MAP_OBJ_FLAG &&
	    MAP_OBJ(pos) <= MAP_OBJ_CASTLE) {
		char *value = load_text_get_setting(section, "object_index");
		if (value == NULL) return -1;
		tiles[pos].u.index = atoi(value);
	} else {
		uint idle_serf = 0;
		uint player = 0;
		uint fish = 0;
		uint resource_type = 0;
		uint resource_amount = 0;

		list_foreach(&section->settings, elm) {
			setting_t *s = (setting_t *)elm;
			if (!strcmp(s->key, "idle_serf")) {
				idle_serf = atoi(s->value);
			} else if (!strcmp(s->key, "player")) {
				player = atoi(s->value);
			} else if (!strcmp(s->key, "fish")) {
				fish = atoi(s->value);
			} else if (!strcmp(s->key, "resource.type")) {
				resource_type = atoi(s->value);
			} else if (!strcmp(s->key, "resource.amount")) {
				resource_amount = atoi(s->value);
			}
		}

		tiles[pos].u.s.field_1 = ((idle_serf & 1) << 7) |
			(player & 3);
		if (MAP_DEEP_WATER(pos)) {
			tiles[pos].u.s.resource = fish;
		} else {
			tiles[pos].u.s.resource = ((resource_type & 7) << 5) |
				(resource_amount & 0x1f);
		}
	}

	return 0;
}

static int
load_text_map_state(list_t *sections)
{
	list_elm_t *elm;
	list_foreach(sections, elm) {
		section_t *s = (section_t *)elm;
		if (!strcmp(s->name, "map")) {
			int r = load_text_map_section(s);
			if (r < 0) return -1;
		}
	}

	return 0;
}

int
load_text_state(FILE *f)
{
	int r;

	list_t sections;
	list_init(&sections);

	load_text_parse(f, &sections);

	r = load_text_global_state(&sections);
	if (r < 0) goto error;

	r = load_text_player_state(&sections);
	if (r < 0) goto error;

	r = load_text_flag_state(&sections);
	if (r < 0) goto error;

	r = load_text_building_state(&sections);
	if (r < 0) goto error;

	r = load_text_inventory_state(&sections);
	if (r < 0) goto error;

	r = load_text_serf_state(&sections);
	if (r < 0) goto error;

	r = load_text_map_state(&sections);
	if (r < 0) goto error;

	globals.game_speed = 0;
	globals.game_speed_save = DEFAULT_GAME_SPEED;

	load_text_free_sections(&sections);
	return 0;

error:
	load_text_free_sections(&sections);
	return -1;
}
