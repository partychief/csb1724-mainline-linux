/*
 * arch/arm/mach-kirkwood/csb1724-setup.c
 *
 * Boards based on the Cogent CSB1724 System on Module
 * (e.g. fieldcloud ns-box rev1)
 *
 * Copyright (C) 2012 Matthew D. Smith (mds@fieldcloud.com)
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/mtd/partitions.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx_eth.h>
#include <linux/ethtool.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <net/dsa.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <plat/pcie.h>
#include "common.h"
#include "mpp.h"

static struct mtd_partition csb1724_nand_parts[] = {
	{
		.name = "u-boot",
		.offset = 0,
		.size = SZ_1M
	}, {
		.name = "uImage",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_4M
	}, {
		.name = "root",
		.offset = MTDPART_OFS_NXTBLK,
		.size = MTDPART_SIZ_FULL
	},
};

static struct mv643xx_eth_platform_data csb1724_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0),
};

/* fieldcloud ns-box CSB1724 GE01 wired to Port 4 on 88E6165 DSA switch chip */

static struct mv643xx_eth_platform_data csb1724_ge01_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(1),
	.speed		= SPEED_1000,
	.duplex		= DUPLEX_FULL,
};

/* 88E6165 Port assignments on fieldcloud ns-box */

static struct dsa_chip_data csb1724_switch_chip_data = {
	.port_names[0]	= "lan1",
	.port_names[1]	= "lan2",
	.port_names[2]	= "lan3",
	.port_names[3]	= "lan4",
	.port_names[4]	= "cpu",
};

static struct dsa_platform_data csb1724_switch_plat_data = {
	.nr_chips	= 1,
	.chip		= &csb1724_switch_chip_data,
};

static struct mv_sata_platform_data csb1724_sata_data = {
	.n_ports	= 2,
};

static unsigned int csb1724_mpp_config[] __initdata = {
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP43_GPIO,		/* GPIO Discrete Relay Input 0 */
	MPP45_GPIO,		/* GPIO Discrete Relay Output 0 */
	MPP46_GPIO,		/* GPIO Discrete Relay Output 1 */
	MPP48_GPIO,		/* GPIO Discrete Relay Output 2 */
	MPP47_GPIO,		/* GPIO Discrete Relay Output 3 */
	MPP39_GPIO,		/* H_RLED */
	MPP41_GPIO,		/* H_RLED */
	MPP40_GPIO,		/* U_BLED */
	0
};

static struct gpio_led csb1724_led_pins[] = {
	{
		.name			= "csb1724:red:health",
		.gpio			= 39,
		.active_low		= 1,
	},
	{
		.name			= "csb1724:red:health",
		.gpio			= 41,
		.active_low		= 1,
	},
	{
		.name			= "csb1724:blue:uid",
		.gpio			= 40,
		.active_low		= 1,
	},
};

static struct gpio_led_platform_data csb1724_led_data = {
	.leds		= csb1724_led_pins,
	.num_leds	= ARRAY_SIZE(csb1724_led_pins),
};

static struct platform_device csb1724_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &csb1724_led_data,
	}
};

static void __init csb1724_init(void)
{
	
	u32 dev, rev;
	
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();
	kirkwood_mpp_conf(csb1724_mpp_config);
	kirkwood_uart0_init();
	kirkwood_nand_init(ARRAY_AND_SIZE(csb1724_nand_parts), 25);
	kirkwood_ehci_init();
	kirkwood_pcie_id(&dev, &rev);
	kirkwood_ge00_init(&csb1724_ge00_data);
	kirkwood_ge01_init(&csb1724_ge01_data);
/*	csb1724_switch_chip_data.sw_addr = 10; */
/*	kirkwood_ge00_switch_init(&csb1724_switch_plat_data, NO_IRQ); */
	kirkwood_sata_init(&csb1724_sata_data);
/*	platform_device_register(&csb1724_leds); */
}

static int __init csb1724_pci_init(void)
{
	if (machine_is_csb1724()) {
		u32 dev, rev;

		kirkwood_pcie_id(&dev, &rev);
		if (dev == MV88F6282_DEV_ID)
			kirkwood_pcie_init(KW_PCIE1 | KW_PCIE0);
		else
			kirkwood_pcie_init(KW_PCIE0);
	}
	return 0;
}
subsys_initcall(csb1724_pci_init);

MACHINE_START(CSB1724, "Cogent CSB1724 SoM")
	/* Maintainer: Matthew D. Smith (mds@fieldcloud.com) */
	.atag_offset	= 0x100,
	.init_machine	= csb1724_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.timer		= &kirkwood_timer,
MACHINE_END
