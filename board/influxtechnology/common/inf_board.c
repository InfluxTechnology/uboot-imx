/*
 * Copyright (C) 2019 Influx Technology
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include "inf_eeprom.h"

int inf_print_board(void)
{
	inf_eeprom_config_t config;

	printf("Board: Influx Technology rexgen smart");
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

