/*
 * Copyright (C) 2019 Influx Technology
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __INF_COMMON_H
#define __INF_COMMON_H

#define INF_CONFIG_MAGIC 0xEA534852

/*
 * Confguration data setup by SPL and given to u-boot
 * via shared memory (INF_SHARED_CONFIG_MEM)
 *
 */
typedef struct {
	uint32_t magic;
	uint32_t ddr_size;	/* size in MB */
	bool is_carrier_v2;	/* true if a V2 carrier is detected */
} inf_config_t;

/*
 * Print information about the COM board
 */
int inf_print_board(void);

/*
 * Load MAC addresses from eeprom
 */
int inf_load_ethaddr(void);

/*
 * Configure TFP410
 */
int inf_configure_tfp410(void);

#endif

