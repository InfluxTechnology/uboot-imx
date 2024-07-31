/*
 * Copyright (C) 2015 Embedded Artists AB
 *
 * Configuration parameters stored in EEPROM for the Embedded Artists
 * i.MX COM Board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <dm.h>

#include "inf_eeprom.h"

#ifdef CONFIG_DM_I2C
static int inf_dm_i2c_init(struct udevice **i2c_dev)
{
	struct udevice *bus;
        int ret;

        ret = uclass_get_device_by_seq(UCLASS_I2C, INF_EEPROM_I2C_BUS, &bus);
        if (ret) {
                printf("%s: Can't find bus\n", __func__);
                return -EINVAL;
        }

        ret = dm_i2c_probe(bus, INF_EEPROM_I2C_SLAVE, 0, i2c_dev);
        if (ret) {
                printf("%s: Can't find device id=0x%x\n",
                        __func__, INF_EEPROM_I2C_SLAVE);
                return -ENODEV;
        }

	return i2c_set_chip_offset_len(*i2c_dev, 2);

}
#endif


int inf_eeprom_init(void)
{
#if !defined(CONFIG_DM_I2C)
	i2c_set_bus_num(INF_EEPROM_I2C_BUS);
	i2c_init(CONFIG_SYS_I2C_SPEED, INF_EEPROM_I2C_SLAVE);
#endif

	return 0;
}

int inf_eeprom_get_config(inf_eeprom_config_t* config)
{
#if !defined(CONFIG_DM_I2C)

	i2c_set_bus_num(INF_EEPROM_I2C_BUS);

	if (i2c_probe(INF_EEPROM_I2C_SLAVE)) {
		return -ENODEV;
	}

	if (i2c_read(INF_EEPROM_I2C_SLAVE,
		0,
		2,
		(uint8_t *)config,
		sizeof(inf_eeprom_config_t)))
	{
		return -EIO;
	}
#else
	struct udevice *i2c_dev = NULL;
	int ret;

	ret = inf_dm_i2c_init(&i2c_dev);
	if (ret) {
		return ret;
	}

        ret = dm_i2c_read(i2c_dev, 0, (uint8_t *)config, sizeof(inf_eeprom_config_t));
        if (ret) {
                printf("%s dm_i2c_read failed, err %d\n", __func__, ret);
                return -EIO;
        }

#endif

	if (config->magic != INF_EEPROM_MAGIC) {
		printf("EA config: invalid magic number\n");
		return -EINVAL;
	}

	if (config->version > INF_EEPROM_CFG_VERSION) {
		printf("EA config: Unsupported config version (%d != %d)\n",
			config->version, INF_EEPROM_CFG_VERSION);
		return -EINVAL;
	}

	return 0;
}

int inf_eeprom_ddr_cfg_init(inf_ddr_cfg_t *cfg)
{
	inf_eeprom_config_t config;
	int ret = 0;

	inf_eeprom_init();
	ret = inf_eeprom_get_config(&config);
	if (!ret) {
		cfg->num_pairs = config.data_size;
		cfg->next = 0;
		cfg->ddr_size_mb = config.ddr_size;
	}

	return ret;
}

int inf_eeprom_ddr_cfg_read(inf_ddr_cfg_t *cfg, inf_ddr_cfg_pair_t* pairs,
	int num, int *num_read)
{
	int to_read;

#ifdef CONFIG_DM_I2C
	int ret;
	struct udevice *i2c_dev = NULL;

        ret = inf_dm_i2c_init(&i2c_dev);
        if (ret) {
                return ret;
        }

#endif

	*num_read = 0;

	/* max to read */
	to_read = cfg->num_pairs - cfg->next;

	/* no more to read */
	if (to_read <= 0) return 0;

	/* fewer requested */
	if (num < to_read) to_read = num;


#if !defined(CONFIG_DM_I2C)
	inf_eeprom_init();
	if (i2c_read(INF_EEPROM_I2C_SLAVE,
		sizeof(inf_eeprom_config_t)+cfg->next*sizeof(inf_ddr_cfg_pair_t),
		2,
		(uint8_t *)pairs,
		to_read*sizeof(inf_ddr_cfg_pair_t)))
	{
		return -EIO;
	}
#else
	ret = dm_i2c_read(i2c_dev,
		sizeof(inf_eeprom_config_t)+cfg->next*sizeof(inf_ddr_cfg_pair_t),
		(uint8_t *)pairs,
		to_read*sizeof(inf_ddr_cfg_pair_t));
	if (ret) {
		printf("%s dm_i2c_read failed, err %d\n", __func__, ret);
		return -EIO;
	}
#endif

	*num_read = to_read;
	cfg->next += to_read;

	return 0;
}

int inf_eeprom_read_all_data(uint8_t* buf, int buf_sz, int *read)
{
#define MAX_BLOCK_LEN (100)
	int to_read;
#if defined(CONFIG_TARGET_MX93INF_UCOM)
	int bytes_read = 0;
#endif

        inf_eeprom_config_t config;
        int ret = 0;

        inf_eeprom_init();
        ret = inf_eeprom_get_config(&config);
        if (ret) return ret;

#ifdef CONFIG_DM_I2C
	struct udevice *i2c_dev = NULL;

        ret = inf_dm_i2c_init(&i2c_dev);
        if (ret) return ret;

#endif

	*read = 0;

	to_read = config.data_size;
	if (buf_sz < to_read) return -EINVAL;


#if !defined(CONFIG_DM_I2C)

	if (i2c_read(INF_EEPROM_I2C_SLAVE,
		sizeof(inf_eeprom_config_t),
		2,
		buf,
		to_read))
	{
		return -EIO;
	}
#else


#if defined(CONFIG_TARGET_MX93INF_UCOM)

	while(bytes_read < to_read) {
		int remaining = to_read - bytes_read;
		int bytes_to_read = remaining > MAX_BLOCK_LEN ? MAX_BLOCK_LEN : remaining;

		ret = dm_i2c_read(i2c_dev,
			sizeof(inf_eeprom_config_t)+bytes_read,
			buf + bytes_read,
			bytes_to_read);
		if (ret) {
			printf("%s dm_i2c_read failed, err %d\n", __func__, ret);
			return -EIO;
		}

		bytes_read += bytes_to_read;
	}

#else

	ret = dm_i2c_read(i2c_dev,
		sizeof(inf_eeprom_config_t),
		buf,
		to_read);
	if (ret) {
		printf("%s dm_i2c_read failed, err %d\n", __func__, ret);
		return -EIO;
	}
#endif
#endif

	*read = to_read;

	return 0;
}
