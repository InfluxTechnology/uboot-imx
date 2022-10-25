// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <common.h>
#include <command.h>
#include <fdt_support.h>
#include <net.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_dtsec.h>
#include <asm/fsl_serdes.h>

int board_eth_init(struct bd_info *bis)
{
#if defined(CONFIG_FMAN_ENET)
	int i, interface;
	struct memac_mdio_info dtsec_mdio_info;
	struct memac_mdio_info tgec_mdio_info;
	struct mii_dev *dev;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 srds_s1;

	srds_s1 = in_be32(&gur->rcwsr[4]) &
					FSL_CORENET2_RCWSR4_SRDS1_PRTCL;
	srds_s1 >>= FSL_CORENET2_RCWSR4_SRDS1_PRTCL_SHIFT;

	dtsec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_DTSEC_MDIO_ADDR;

	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fm_memac_mdio_init(bis, &dtsec_mdio_info);

	tgec_mdio_info.regs =
		(struct memac_mdio_controller *)CONFIG_SYS_FM1_TGEC_MDIO_ADDR;
	tgec_mdio_info.name = DEFAULT_FM_TGEC_MDIO_NAME;

	/* Register the 10G MDIO bus */
	fm_memac_mdio_init(bis, &tgec_mdio_info);

	/* Set the two on-board RGMII PHY address */
	fm_info_set_phy_address(FM1_DTSEC3, RGMII_PHY1_ADDR);
	fm_info_set_phy_address(FM1_DTSEC4, RGMII_PHY2_ADDR);

	switch (srds_s1) {
	case 0x66:
	case 0x6b:
		fm_info_set_phy_address(FM1_10GEC1, CORTINA_PHY_ADDR1);
		fm_info_set_phy_address(FM1_10GEC2, CORTINA_PHY_ADDR2);
		fm_info_set_phy_address(FM1_10GEC3, FM1_10GEC3_PHY_ADDR);
		fm_info_set_phy_address(FM1_10GEC4, FM1_10GEC4_PHY_ADDR);
		break;
	default:
		printf("SerDes1 protocol 0x%x is not supported on T208xRDB\n",
		       srds_s1);
		break;
	}

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		interface = fm_info_get_enet_if(i);
		switch (interface) {
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_TXID:
		case PHY_INTERFACE_MODE_RGMII_RXID:
		case PHY_INTERFACE_MODE_RGMII_ID:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

	for (i = FM1_10GEC1; i < FM1_10GEC1 + CONFIG_SYS_NUM_FM1_10GEC; i++) {
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_XGMII:
			dev = miiphy_get_dev_by_name(DEFAULT_FM_TGEC_MDIO_NAME);
			fm_info_set_mdio(i, dev);
			break;
		default:
			break;
		}
	}

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}

/* Disable the MAC5 and MAC6 "fsl,fman-memac" nodes and the two
 * "fsl,dpa-ethernet" nodes that reference them.
 */
void fdt_fixup_board_fman_ethernet(void *fdt)
{
	int mac_off, eth_off, i;
	char mac_path[2][42] = {
		"/soc@ffe000000/fman@400000/ethernet@e8000",
		"/soc@ffe000000/fman@400000/ethernet@ea000",
	};
	u32 eth_ph;

	for (i = 0; i < 2; i++) {
		/* Disable the MAC node */
		mac_off = fdt_path_offset(fdt, mac_path[i]);
		if (mac_off < 0)
			continue;
		fdt_status_disabled(fdt, mac_off);

		/* Disable the fsl,dpa-ethernet node that points to the MAC.
		 * The fsl,fman-mac property refers to the MAC's phandle.
		 */
		eth_ph = fdt_get_phandle(fdt, mac_off);
		if (eth_ph <= 0)
			continue;

		eth_off = fdt_node_offset_by_prop_value(fdt, -1, "fsl,fman-mac",
							&eth_ph,
							sizeof(eth_ph));
		if (eth_off >= 0)
			fdt_status_disabled(fdt, eth_off);
	}
}

void fdt_fixup_board_enet(void *fdt)
{
	return;
}
