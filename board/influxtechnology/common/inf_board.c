/*
 * Copyright (C) 2019 Embedded Artists AB
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <env.h>
#include "inf_eeprom.h"

static char inf_board_part[9];
static char inf_board_batch[9];

int inf_print_board(void)
{
	inf_eeprom_config_t config;

	printf("Board: Embedded Artists ");
	if (inf_eeprom_get_config(&config) == 0) {

		printf("%s\n", config.name);
		printf("       %05d, %s, WO%d\n",
			config.board_part_nr,
			config.board_rev,
			config.batch);
	}
	else {
		printf(" [Unknown board due to invalid configuration data]\n");
	}

	return 0;
}

int inf_board_info_to_env(void)
{
	inf_eeprom_config_t config;

	if (inf_eeprom_get_config(&config) == 0) {

		sprintf(inf_board_part, "%d", config.board_part_nr);
		env_set("board_part", inf_board_part);
		sprintf(inf_board_batch, "%d", config.batch);
		env_set("board_batch", inf_board_batch);
		env_set("board_rev", (const char*)config.board_rev);
		env_save();
	}

	return 0;
}

