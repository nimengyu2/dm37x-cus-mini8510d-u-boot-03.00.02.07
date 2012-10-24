/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from SBC8150 Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "sbc8150.h"
#include <netdev.h>
#include <asm/arch/mem.h>

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_SBC8150;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

static void setup_net_chip(void);

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

//	MUX_SBC8150_XM();
	/* Set VAUX2 to 1.8V for EHCI PHY */
	twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
				TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
				TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
				TWL4030_PM_RECEIVER_DEV_GRP_P1);
	setenv("mpurate", "1000");

	/* Configure GPIOs to output */
	writel(~(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1), &gpio6_base->oe);
	writel(~(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12), &gpio5_base->oe);

	/* Set GPIOs */
	writel(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1,
		&gpio6_base->setdataout);
	writel(GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 |
		GPIO15 | GPIO14 | GPIO13 | GPIO12, &gpio5_base->setdataout);

	dieid_num_r();
	setup_net_chip();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_SBC8150();
}

static void setup_net_chip(void)
{
#ifdef CONFIG_DRIVER_DM9000
        struct ctrl_id *id_base = (struct ctrl_id *)OMAP34XX_ID_L4_IO_BASE;
        uchar enetaddr[6];
        u32 die_id_0;
        /* Configure GPMC registers for DM9000 */
        writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[6].config1);
        writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[6].config2);
        writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[6].config3);
        writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[6].config4);
        writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[6].config5);
        writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[6].config6);
        writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[6].config7);

        /* Use OMAP DIE_ID as MAC address */
        if (!eth_getenv_enetaddr("ethaddr", enetaddr)) {
                printf("ethaddr not set, using Die ID\n");
                die_id_0 = readl(&id_base->die_id_0);
                enetaddr[0] = 0x02; /* locally administered */
                enetaddr[1] = readl(&id_base->die_id_1) & 0xff;
                enetaddr[2] = (die_id_0 & 0xff000000) >> 24;
                enetaddr[3] = (die_id_0 & 0x00ff0000) >> 16;
                enetaddr[4] = (die_id_0 & 0x0000ff00) >> 8;
                enetaddr[5] = (die_id_0 & 0x000000ff);
                eth_setenv_enetaddr("ethaddr", enetaddr);
        }
#endif
}

int board_eth_init(bd_t *bis)
{
        int rc = 0;
#ifdef CONFIG_DRIVER_DM9000
        rc = dm9000_initialize(bis);
#endif

        return rc;
}
