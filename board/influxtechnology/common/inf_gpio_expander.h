/*
 * Copyright (C) 2019 Influx Technology
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __INF_GPIO_EXPANDER_H
#define __INF_GPIO_EXPANDER_H

int inf_gpio_exp_configure(int i2c_bus);
int inf_get_carrier_board_version(int i2c_bus);
bool inf_is_carrier_v2(int i2c_bus);
bool inf_is_carrier_v3(int i2c_bus);

#endif

