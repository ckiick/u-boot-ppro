#include <common.h>
#include <miiphy.h>
#include <asm/arch-oxnas820/regs.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;
#define FLASH_WORD_SIZE unsigned short

#define STATIC_BUS_FLASH_CONFIG 0x4f1f3f3f  /* Slow ASIC settings */

int board_init(void)
{
    gd->bd->bi_arch_number = MACH_TYPE_OXNAS;
    gd->bd->bi_boot_params = PHYS_SDRAM_1_PA + 0x100;
/*    gd->flags = 0; */

    icache_enable();

#if 0
    /* Block reset Static core */
    *(volatile u32*)SYS_CTRL_RSTEN_SET_CTRL = (1UL << SYS_CTRL_RSTEN_STATIC_BIT);
    *(volatile u32*)SYS_CTRL_RSTEN_CLR_CTRL = (1UL << SYS_CTRL_RSTEN_STATIC_BIT);

    /* Enable clock to Static core */
    *(volatile u32*)SYS_CTRL_CKEN_SET_CTRL = (1UL << SYS_CTRL_CKEN_STATIC_BIT);

	/* enable flash support on static bus.
     * Enable static bus onto GPIOs, only CS0 */
    *(volatile u32*)SYS_CTRL_SECONDARY_SEL |= 0x01FFF000;

    /* Setup the static bus CS0 to access FLASH */
    *(volatile u32*)STATIC_CONTROL_BANK0 = STATIC_BUS_FLASH_CONFIG;
    
    {
        unsigned int pins;
        /* Reset UART-1 */
        pins = 1 << SYS_CTRL_RSTEN_UART1_BIT;
       *(volatile u32*)SYS_CTRL_RSTEN_SET_CTRL = pins;
       udelay(100);
       *(volatile u32*)SYS_CTRL_RSTEN_CLR_CTRL = pins;
       udelay(100);

        /* Setup pin mux'ing for UART2 */
        pins = (1 << UART_1_SIN_MF_PIN) | (1 << UART_1_SOUT_MF_PIN);
        *(volatile u32*)SYS_CTRL_SECONDARY_SEL &= ~pins;
        *(volatile u32*)SYS_CTRL_TERTIARY_SEL  &= ~pins;
        *(volatile u32*)SYS_CTRL_DEBUG_SEL  &=  ~pins;
        *(volatile u32*)SYS_CTRL_ALTERNATIVE_SEL  |= pins; // Route UART1 SOUT and SIN onto external pins
    }
#endif

	return 0;
}

/* OX820 code */
void reset_cpu(ulong addr)
{
	printf("Resetting...\n");

    // Assert reset to cores as per power on defaults
    // Don't touch the DDR interface as things will come to an impromptu stop
	// NB Possibly should be asserting reset for PLLB, but there are timing
	//    concerns here according to the docs
    *(volatile u32*)SYS_CTRL_RSTEN_SET_CTRL =
        (1UL << SYS_CTRL_RSTEN_COPRO_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_USBHS_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_USBHSPHYA_BIT ) |
        (1UL << SYS_CTRL_RSTEN_MACA_BIT      ) |
        (1UL << SYS_CTRL_RSTEN_PCIEA_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_SGDMA_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_CIPHER_BIT    ) |
        (1UL << SYS_CTRL_RSTEN_SATA_BIT      ) |
        (1UL << SYS_CTRL_RSTEN_SATA_LINK_BIT ) |
        (1UL << SYS_CTRL_RSTEN_SATA_PHY_BIT  ) |
        (1UL << SYS_CTRL_RSTEN_PCIEPHY_BIT   ) |
        (1UL << SYS_CTRL_RSTEN_STATIC_BIT    ) |
        (1UL << SYS_CTRL_RSTEN_UART1_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_UART2_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_MISC_BIT      ) |
        (1UL << SYS_CTRL_RSTEN_I2S_BIT       ) |
        (1UL << SYS_CTRL_RSTEN_SD_BIT        ) |
        (1UL << SYS_CTRL_RSTEN_MACB_BIT      ) |
        (1UL << SYS_CTRL_RSTEN_PCIEB_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_VIDEO_BIT     ) |
        (1UL << SYS_CTRL_RSTEN_USBHSPHYB_BIT ) |
        (1UL << SYS_CTRL_RSTEN_USBDEV_BIT    );

    // Release reset to cores as per power on defaults
    *(volatile u32*)SYS_CTRL_RSTEN_CLR_CTRL = (1UL << SYS_CTRL_RSTEN_GPIO_BIT);

    // Disable clocks to cores as per power-on defaults - must leave DDR
	// related clocks enabled otherwise we'll stop rather abruptly.
    *(volatile u32*)SYS_CTRL_CKEN_CLR_CTRL =
        (1UL << SYS_CTRL_CKEN_COPRO_BIT) 	|
        (1UL << SYS_CTRL_CKEN_DMA_BIT)   	|
        (1UL << SYS_CTRL_CKEN_CIPHER_BIT)	|
        (1UL << SYS_CTRL_CKEN_SD_BIT)  		|
        (1UL << SYS_CTRL_CKEN_SATA_BIT)  	|
        (1UL << SYS_CTRL_CKEN_I2S_BIT)   	|
        (1UL << SYS_CTRL_CKEN_USBHS_BIT) 	|
        (1UL << SYS_CTRL_CKEN_MAC_BIT)   	|
        (1UL << SYS_CTRL_CKEN_PCIEA_BIT)   	|
        (1UL << SYS_CTRL_CKEN_STATIC_BIT)	|
        (1UL << SYS_CTRL_CKEN_MACB_BIT)		|
        (1UL << SYS_CTRL_CKEN_PCIEB_BIT)	|
        (1UL << SYS_CTRL_CKEN_REF600_BIT)	|
        (1UL << SYS_CTRL_CKEN_USBDEV_BIT);

    // Enable clocks to cores as per power-on defaults

    // Set sys-control pin mux'ing as per power-on defaults
    *(volatile u32*)SYS_CTRL_SECONDARY_SEL 	 = 0x0UL;
    *(volatile u32*)SYS_CTRL_TERTIARY_SEL 	 = 0x0UL;
    *(volatile u32*)SYS_CTRL_QUATERNARY_SEL  = 0x0UL;
    *(volatile u32*)SYS_CTRL_DEBUG_SEL  	 = 0x0UL;
    *(volatile u32*)SYS_CTRL_ALTERNATIVE_SEL = 0x0UL;
    *(volatile u32*)SYS_CTRL_PULLUP_SEL 	 = 0x0UL;

    *(volatile u32*)SEC_CTRL_SECONDARY_SEL 	 = 0x0UL;
    *(volatile u32*)SEC_CTRL_TERTIARY_SEL 	 = 0x0UL;
    *(volatile u32*)SEC_CTRL_QUATERNARY_SEL  = 0x0UL;
    *(volatile u32*)SEC_CTRL_DEBUG_SEL  	 = 0x0UL;
    *(volatile u32*)SEC_CTRL_ALTERNATIVE_SEL = 0x0UL;
    *(volatile u32*)SEC_CTRL_PULLUP_SEL 	 = 0x0UL;

    // No need to save any state, as the ROM loader can determine whether reset
    // is due to power cycling or programatic action, just hit the (self-
    // clearing) CPU reset bit of the block reset register
    *(volatile u32*)SYS_CTRL_RSTEN_SET_CTRL = 
		(1UL << SYS_CTRL_RSTEN_SCU_BIT) |
		(1UL << SYS_CTRL_RSTEN_ARM0_BIT) |
		(1UL << SYS_CTRL_RSTEN_ARM1_BIT);
}

extern int oxnas_gmac_register(bd_t *bis);

/* all we do here is call the driver init function for each nic we have */
int board_eth_init(bd_t *bis)
{
	return oxnas_gmac_register(bis);
}

