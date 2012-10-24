#include <common.h>
#include <net.h>
#include <command.h>
#include <config_cmd_default.h>
#include <asm/io.h>
 
#define all 0x00
 
int do_led(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])  
{ 
	struct gpio *gpio5_base = (struct gpio *)OMAP34XX_GPIO5_BASE;
	struct gpio *gpio6_base = (struct gpio *)OMAP34XX_GPIO6_BASE;

	switch(argc) { 
	case 3:
		if(strcmp(argv[1], "flash") == 0) {
			if(strcmp(argv[2], "all") == 0) {
				while(1){ 
#if defined (CONFIG_OMAP3_DEVKIT8500)
        				/* Configure GPIOs to output */
        				writel(~GPIO26, &gpio6_base->oe);	
					/* Set GPIOs */
					writel(GPIO26, &gpio6_base->setdataout);	
					udelay(500000);
					/* Clear GPIOs */
					writel(GPIO26, &gpio6_base->cleardataout);
					udelay(500000);
#elif defined (CONFIG_DM3730_EVK)
                                        /* Configure GPIOs to output */
                                        writel(~GPIO26, &gpio6_base->oe);
                                        /* Set GPIOs */
                                        writel(GPIO26, &gpio6_base->setdataout);
                                        udelay(500000);
                                        /* Clear GPIOs */
                                        writel(GPIO26, &gpio6_base->cleardataout);
                                        udelay(500000);
#elif defined (CONFIG_OMAP3_SBC8510)
                                        /* Configure GPIOs to output */
                                        writel(~(GPIO8 | GPIO9 | GPIO10 | GPIO11), &gpio5_base->oe);
                                        /* Set GPIOs */
                                        writel((GPIO8 | GPIO9 | GPIO10 | GPIO11), &gpio5_base->setdataout);
                                        udelay(500000);
                                        /* Clear GPIOs */
                                        writel((GPIO8 | GPIO9 | GPIO10 | GPIO11), &gpio5_base->cleardataout);
                                        udelay(500000);
#elif defined (CONFIG_OMAP3_SBC8100_PLUS)
                                        /* Configure GPIOs to output */
                                        writel(~(GPIO8 | GPIO9 | GPIO10 | GPIO11), &gpio5_base->oe);
                                        /* Set GPIOs */
                                        writel((GPIO8 | GPIO9 | GPIO10 | GPIO11), &gpio5_base->setdataout);
                                        udelay(500000);
                                        /* Clear GPIOs */
                                        writel((GPIO8 | GPIO9 | GPIO10 | GPIO11), &gpio5_base->cleardataout);
                                        udelay(500000);
#elif defined (CONFIG_OMAP3_SBC8530)
					u8 byte;

                                        /* Set GPIOs */
                                        byte = 0x33;
                                        i2c_write(0x4A, 0xEE, 1, &byte, 1);//LEDA enable
                                        udelay(500000);
                                        /* Clear GPIOs */
                                        byte = 0x00;
                                        i2c_write(0x4A, 0xEE, 1, &byte, 1);//LEDA disable
                                        udelay(500000);
#elif defined (CONFIG_OMAP3_SBC8150)
					/* Configure GPIOs to output */
					writel(~GPIO22, &gpio6_base->oe);
                                        /* Set GPIOs */
                                        writel(GPIO22, &gpio6_base->setdataout);
                                        udelay(500000);
                                        /* Clear GPIOs */
                                        writel(GPIO22, &gpio6_base->cleardataout);
                                        udelay(500000);

#endif
				  }
		        }
		}
		break;
	case 2:
		if(strcmp(argv[1], "on") == 0) {
#if defined (CONFIG_OMAP3_SBC8150)
			/* Configure GPIOs to output */
			writel(~GPIO22, &gpio6_base->oe);
			/* Set GPIOs */
			writel(GPIO22, &gpio6_base->cleardataout);
#endif
		} else if(strcmp(argv[1], "off") == 0) {
#if defined (CONFIG_OMAP3_SBC8150)
                        /* Configure GPIOs to output */
                        writel(~GPIO22, &gpio6_base->oe);
                        /* Set GPIOs */
                        writel(GPIO22, &gpio6_base->setdataout);
#endif
		}
		break;
	
	case 1:          
	default:  
		printf ("Number of arguments  must be 2:\n");
	}    
}

U_BOOT_CMD(
        led ,      3,      1,      do_led,
        "    ----Just test for led\n",
        "[flash] [all]\n"
);


