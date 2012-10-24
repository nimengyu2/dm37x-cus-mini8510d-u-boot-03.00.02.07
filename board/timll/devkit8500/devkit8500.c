/*
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Devkit8500 Board and 3430 SDP code by
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
#include "devkit8500.h"
#include <netdev.h>
#include <asm/arch/mem.h>

#include "devkit_logo_FNL_2.h"

static int devkit8500_revision;

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_DEVKIT8500;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: devkit8500_get_revision
 * Description: Return the revision of the Devkit8500Board this code is running on.
 */
int devkit8500_get_revision(void)
{
	return devkit8500_revision;
}

/*
 * Routine: devkit8500_identify
 * Description: Detect if we are running on a Devkit8500 revision Ax/Bx,
 *		C1/2/3, C4 or D. This can be done by reading
 *		the level of GPIO173, GPIO172 and GPIO171. This should
 *		result in
 *		GPIO173, GPIO172, GPIO171: 1 1 1 => Ax/Bx
 *		GPIO173, GPIO172, GPIO171: 1 1 0 => C1/2/3
 *		GPIO173, GPIO172, GPIO171: 1 0 1 => C4
 *		GPIO173, GPIO172, GPIO171: 0 0 0 => XM
 */
void devkit8500_identify(void)
{
#if 0
	omap_request_gpio(171);
	omap_request_gpio(172);
	omap_request_gpio(173);
	omap_set_gpio_direction(171, 1);
	omap_set_gpio_direction(172, 1);
	omap_set_gpio_direction(173, 1);

	devkit8500_revision = omap_get_gpio_datain(173) << 2 |
			  omap_get_gpio_datain(172) << 1 |
			  omap_get_gpio_datain(171);
	omap_free_gpio(171);
	omap_free_gpio(172);
	omap_free_gpio(173);
#else
	devkit8500_revision = REVISION_XM;
#endif
}

static void setup_net_chip(void);
static void dss_init(void);

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

	devkit8500_identify();

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

	switch (devkit8500_revision) {
	case REVISION_AXBX:
		printf("Devkit8500 Rev Ax/Bx\n");
		setenv("mpurate", "600");
		break;
	case REVISION_CX:
		printf("Devkit8500 Rev C1/C2/C3\n");
		MUX_DEVKIT8500_C();
		setenv("mpurate", "600");
		break;
	case REVISION_C4:
		printf("Devkit8500 Rev C4\n");
		MUX_DEVKIT8500_C();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		setenv("mpurate", "720");
		break;
	case REVISION_XM:
		printf("Devkit8500 xM Rev A\n");
//		MUX_DEVKIT8500_XM();
		/* Set VAUX2 to 1.8V for EHCI PHY */
		twl4030_pmrecv_vsel_cfg(TWL4030_PM_RECEIVER_VAUX2_DEDICATED,
					TWL4030_PM_RECEIVER_VAUX2_VSEL_18,
					TWL4030_PM_RECEIVER_VAUX2_DEV_GRP,
					TWL4030_PM_RECEIVER_DEV_GRP_P1);
		setenv("mpurate", "1000");
		break;
	default:
		printf("Devkit8500 unknown 0x%02x\n", devkit8500_revision);
	}

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

	dss_init();

#ifdef AUTO_UPDATESYS	
	run_command("run updatesys", 0);
#endif
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
	MUX_DEVKIT8500();
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

static void dss_init(void)
{
	unsigned int i;
       	unsigned char pixel[3];
       	int offset = 0;
	unsigned char byte;

        /* enable dvi output */
        byte = 0x80;
        i2c_write(0x49, 0x9B, 1, &byte, 1);
        i2c_write(0x49, 0x9E, 1, &byte, 1);

       /* assuming a resolution of 1280x720 - draw logo into dss buffer */

       /* fill in the blank white bar at the top */
       for(i = 0; i<(((720/2)-(height/2))*1280)*2; i+=2) {
               *((unsigned short *)(0x80500000 + i + offset)) = 0xffff;
       }

       offset += i;

       /* Add the image data */
       for (i = 0; i < ((width*height)*2);) {
               HEADER_PIXEL(header_data, pixel);
               *((unsigned short *)(0x80500000 + i + offset)) =
                       ((((pixel[0])&0xf8) << 8) |
                        (((pixel[1])&0xfc) << 3) |
                        (((pixel[2])&0xf8) >> 3));
               i+=2;
       }

       offset += i;

       /* fill in the blank white bar at the bottom */
       for(i = 0; i<(720 - (offset/2/1280))*1280*2; i+=2) {
               *((unsigned short *)(0x80500000 + i + offset)) = 0xffff;
       }

//	*((uint *) 0x48310034) = 0xfefffedf;	//GPIO_OE
//	*((uint *) 0x48310094) = 0x01000120;	//GPIO_SET
#if 0
	*((uint *) 0x48004D44) = 0x0001b00c;	//CM_CLKSEL2_PLL
	*((uint *) 0x48004E40) = 0x00001006;	//CM_CLKSEL_DSS
	*((uint *) 0x48004D00) = 0x00370037;	//CM_CLKEN_PLL
#endif
//	*((uint *) 0x48050C00) = 0x00000002;	//VENC_REV_ID
//	*((uint *) 0x48050C04) = 0x0000001B;	//VENC_STATUS
	*((uint *) 0x48050C08) = 0x00000040;	//VENC_F_CONTROL
//	*((uint *) 0x48050C0C) = 0x00000000;	//??
	*((uint *) 0x48050C10) = 0x00000000;	//VENC_VIDOUT_CTRL
	*((uint *) 0x48050C14) = 0x00008000;	//VENC_SYNC_CTRL
//	*((uint *) 0x48050C18) = 0x00000000;	//??
	*((uint *) 0x48050C1C) = 0x00008359;	//VENC_LLEN
	*((uint *) 0x48050C20) = 0x0000020C;	//VENC_FLENS
	*((uint *) 0x48050C24) = 0x00000000;	//VENC_HFLTR_CTRL
	*((uint *) 0x48050C28) = 0x043F2631;	//VENC_CC_CARR_WSS_CARR
	*((uint *) 0x48050C2C) = 0x00000024;	//VENC_C_PHASE
	*((uint *) 0x48050C30) = 0x00000130;	//VENC_GAIN_U
	*((uint *) 0x48050C34) = 0x00000198;	//VENC_GAIN_V
	*((uint *) 0x48050C38) = 0x000001C0;	//VENC_GAIN_Y
	*((uint *) 0x48050C3C) = 0x0000006A;	//VENC_BLACK_LEVEL
	*((uint *) 0x48050C40) = 0x0000005C;	//VENC_BLANK_LEVEL
	*((uint *) 0x48050C44) = 0x00000000;	//VENC_X_COLOR
	*((uint *) 0x48050C48) = 0x00000001;	//VENC_M_CONTROL
	*((uint *) 0x48050C4C) = 0x0000003F;	//VENC_BSTAMP_WSS_DATA
	*((uint *) 0x48050C50) = 0x21F07C1F;	//VENC_S_CARR
	*((uint *) 0x48050C54) = 0x00000000;	//VENC_LINE21
	*((uint *) 0x48050C58) = 0x00000015;	//VENC_LN_SEL
	*((uint *) 0x48050C5C) = 0x00001400;	//VENC_L21_WC_CTL
	*((uint *) 0x48050C60) = 0x00000000;	//VENC_HTRIGGER_VTRIGGER
	*((uint *) 0x48050C64) = 0x069300F4;	//VENC_SAVID_EAVID
	*((uint *) 0x48050C68) = 0x0016020C;	//VENC_FLEN_FAL
	*((uint *) 0x48050C6C) = 0x00060107;	//VENC_LAL_PHASE_RESET
	*((uint *) 0x48050C70) = 0x008D034E;	//VENC_HS_INT_START_STOP_X
	*((uint *) 0x48050C74) = 0x000F0359;	//VENC_HS_EXT_START_STOP_X
	*((uint *) 0x48050C78) = 0x01A00000;	//VENC_VS_INT_START_X
	*((uint *) 0x48050C7C) = 0x020501A0;	//VENC_VS_INT_STOP_X_VS_INT_START_Y
	*((uint *) 0x48050C80) = 0x01AC0024;	//VENC_VS_INT_STOP_Y_VS_EXT_START_X
	*((uint *) 0x48050C84) = 0x020D01AC;	//VENC_VS_EXT_STOP_X_VS_EXT_START_Y
	*((uint *) 0x48050C88) = 0x00000006;	//VENC_VS_EXT_STOP_Y
//	*((uint *) 0x48050C8C) = 0x00000000;	//??
	*((uint *) 0x48050C90) = 0x03480079;	//VENC_AVID_START_STOP_X
	*((uint *) 0x48050C94) = 0x02040024;	//VENC_AVID_START_STOP_Y
//	*((uint *) 0x48050C98) = 0x00000000;	//??
//	*((uint *) 0x48050C9C) = 0x00000000;	//??
	*((uint *) 0x48050CA0) = 0x0001008A;	//VENC_FID_INT_START_X_FID_INT_START_Y
	*((uint *) 0x48050CA4) = 0x01AC0106;	//VENC_FID_INT_OFFSET_Y_FID_EXT_START_X
	*((uint *) 0x48050CA8) = 0x01060006;	//VENC_FID_EXT_START_Y_FID_EXT_OFFSET_Y
//	*((uint *) 0x48050CAC) = 0x00000000;	//??
	*((uint *) 0x48050CB0) = 0x00140001;	//VENC_TVDETGP_INT_START_STOP_X
	*((uint *) 0x48050CB4) = 0x00010001;	//VENC_TVDETGP_INT_START_STOP_Y
	*((uint *) 0x48050CB8) = 0x00FF0000;	//VENC_GEN_CTRL
//	*((uint *) 0x48050CBC) = 0x00000000;	//??
//	*((uint *) 0x48050CC0) = 0x00000000;	//??
	*((uint *) 0x48050CC4) = 0x0000000D;	//VENC_OUTPUT_CONTROL
	*((uint *) 0x48050CC8) = 0x00000000;	//VENC_OUTPUT_TEST

	*((uint *) 0x48050010) = 0x00000001;	//DSS_SYSCONFIG	
	*((uint *) 0x48050040) = 0x00000078;	//DSS_CONTROL
//	*((uint *) 0x48050044) = 0x00000000;	//??
//	*((uint *) 0x48050048) = 0x00000000;	//??
//	*((uint *) 0x48050050) = 0x00000000;	//??
//	*((uint *) 0x48050058) = 0x00000000;	//??
	*((uint *) 0x48050410) = 0x00002015;	//DISPC_SYSCONFIG
	*((uint *) 0x48050414) = 0x00000001;	//DISPC_SYSSTATUS
	*((uint *) 0x48050444) = 0x00000004;	//DISPC_CONFIG
	*((uint *) 0x4805044c) = 0xFFFFFFFF;	//DISPC_DEFAULT_COLOR_0
	*((uint *) 0x48050450) = 0x00000000;	//DISPC_DEFAULT_COLOR_1
	*((uint *) 0x48050454) = 0x00000000;	//DISPC_TRANS_COLOR_0
	*((uint *) 0x48050458) = 0x00000000;	//DISPC_TRANS_COLOR_1
	*((uint *) 0x48050464) = 0x0ff03f31;	//DISPC_TIMING_H
	*((uint *) 0x48050468) = 0x01400504;	//DISPC_TIMING_V
	*((uint *) 0x4805046c) = 0x00007028;	//DISPC_POL_FREQ
	*((uint *) 0x48050470) = 0x00010002;	//DISPC_DIVISOR
	*((uint *) 0x48050478) = 0x00ef027f;	//DISPC_SIZE_DIG
	*((uint *) 0x4805047c) = 0x02cf04ff;	//DISPC_SIZE_LCD
	*((uint *) 0x48050480) = 0x80500000;	//DISPC_GFX_BA0
	*((uint *) 0x48050484) = 0x80500000;	//DISPC_GFX_BA1
	*((uint *) 0x48050488) = 0x00000000;	//DISPC_GFX_POSITION
	*((uint *) 0x4805048c) = 0x02cf04ff;	//DISPC_GFX_SIZE
	*((uint *) 0x480504a0) = 0x0000008d;	//DISPC_GFX_ATTRIBUTES
	*((uint *) 0x480504a4) = 0x03fc03bc;	//DISPC_GFX_FIFO_THRESHOLD
	*((uint *) 0x480504a8) = 0x00000400;	//DISPC_GFX_FIFO_SIZE_STATUS
	*((uint *) 0x480504ac) = 0x00000001;	//DISPC_GFX_ROW_INC
	*((uint *) 0x480504b0) = 0x00000001;	//DISPC_GFX_PIXEL_INC
	*((uint *) 0x480504b4) = 0x00000000;	//DISPC_GFX_WINDOW_SKIP
	*((uint *) 0x480504b8) = 0x807ff000;	//DISPC_GFX_TABLE_BA
	udelay(1000);
	*((uint *) 0x48050440) = 0x0001836b;	//DISPC_CONTROL
	udelay(1000);	
	*((uint *) 0x48050440) = 0x0001836b;	//DISPC_CONTROL
	udelay(1000);
	*((uint *) 0x48050440) = 0x0001836b;	//DISPC_CONTROL
	udelay(1000);
}

