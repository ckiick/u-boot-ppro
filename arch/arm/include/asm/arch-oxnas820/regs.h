#ifndef __ox820_regs_h
#define __ox820_regs_h

#define PHYS_SDRAM_1_PA         0x60000000  /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_MAX_SIZE	(512 * 1024 * 1024)

#define PHYS_SRAM_BASE 0x50000000
#define PHYS_SRAM_SIZE (64 * 1024)

#define EXCEPTION_BASE	(PHYS_SRAM_BASE)

/* Core addresses */
#define USBHOST_BASE			0x40200000
#define MACA_BASE_PA            0x40400000
#define MACB_BASE_PA            0x40800000
#define MAC_BASE_PA             MACA_BASE_PA
#define STATIC_CS0_BASE_PA      0x41000000
#define STATIC_CS1_BASE_PA      0x41400000
#define STATIC_CONTROL_BASE_PA  0x41C00000
#define SATA_DATA_BASE_PA       0x42000000 /* non-functional, DMA just needs an address */

#define GPIO_1_PA               0x44000000
#define GPIO_2_PA               0x44100000

#define SYS_CONTROL_BASE_PA     0x44e00000
#define SEC_CONTROL_BASE_PA     0x44f00000
#define RPSA_BASE_PA            0x44400000
#define RPSC_BASE_PA            0x44500000

/* Static bus registers */
#define STATIC_CONTROL_VERSION  ((STATIC_CONTROL_BASE_PA) + 0x0)
#define STATIC_CONTROL_BANK0    ((STATIC_CONTROL_BASE_PA) + 0x4)
#define STATIC_CONTROL_BANK1    ((STATIC_CONTROL_BASE_PA) + 0x8)

/* OTP control registers */
#define OTP_ADDR_PROG		(SEC_CONTROL_BASE_PA + 0x1E0)
#define OTP_READ_DATA 		(SEC_CONTROL_BASE_PA + 0x1E4)

/* OTP bit control */
#define OTP_ADDR_MASK    	(0x03FF)
#define OTP_PS			(1<<10)
#define OTP_PGENB		(1<<11)
#define OTP_LOAD		(1<<12)
#define OTP_STROBE		(1<<13)
#define OTP_CSB			(1<<14)
#define FUSE_BLOWN  		1
#define OTP_MAC                 (0x79)
/**
 * Timer
 */
#define CONFIG_SYS_TIMERBASE            ((RPSA_BASE_PA) + 0x200)
#define TIMER_PRESCALE_BIT       2
#define TIMER_PRESCALE_1_ENUM    0
#define TIMER_PRESCALE_16_ENUM   1
#define TIMER_PRESCALE_256_ENUM  2
#define TIMER_MODE_BIT           6
#define TIMER_MODE_FREE_RUNNING  0
#define TIMER_MODE_PERIODIC      1
#define TIMER_ENABLE_BIT         7
#define TIMER_ENABLE_DISABLE     0
#define TIMER_ENABLE_ENABLE      1

#define TIMER_PRESCALE_ENUM      (TIMER_PRESCALE_16_ENUM)

/**
 * GPIO
 */
#define GPIO_1_OE       ((GPIO_1_PA) + 0x4)
#define GPIO_1_SET_OE   ((GPIO_1_PA) + 0x1C)
#define GPIO_1_CLR_OE   ((GPIO_1_PA) + 0x20)

#define GPIO_1_OUT      ((GPIO_1_PA) + 0x10)
#define GPIO_1_SET      ((GPIO_1_PA) + 0x14)
#define GPIO_1_CLR      ((GPIO_1_PA) + 0x18)

#define GPIO_1_DBC      ((GPIO_1_PA) + 0x24)

#define GPIO_1_PULL     ((GPIO_1_PA) + 0x50)
#define GPIO_1_DRN      ((GPIO_1_PA) + 0x54)

#define GPIO_2_OE       ((GPIO_2_PA) + 0x4)
#define GPIO_2_SET_OE   ((GPIO_2_PA) + 0x1C)
#define GPIO_2_CLR_OE   ((GPIO_2_PA) + 0x20)

#define GPIO_2_OUT      ((GPIO_2_PA) + 0x10)
#define GPIO_2_SET      ((GPIO_2_PA) + 0x14)
#define GPIO_2_CLR      ((GPIO_2_PA) + 0x18)

/**
 * Serial Configuration
 */
#define UART_1_BASE (0x44200000)
#define UART_2_BASE (0x44300000)

// #define CONFIG_SYS_NS16550          1
// #define CONFIG_SYS_NS16550_SERIAL   1
// #define CONFIG_SYS_NS16550_REG_SIZE 1

// #define CONFIG_SYS_NS16550_CLK      (RPSCLK)
#define USE_UART_FRACTIONAL_DIVIDER

/* UART_A multi function pins (in sys ctrl core)*/
#define UART_1_SIN_MF_PIN              30
#define UART_1_SOUT_MF_PIN             31

#define UART_2_SIN_MF_PIN              14
#define UART_2_SOUT_MF_PIN             15

/* Eth A multi function pins */
#define MACA_MDC_MF_PIN                3
#define MACA_MDIO_MF_PIN               4

/**
 * System block reset and clock control
 */
#define SYS_CTRL_PCI_STAT               (SYS_CONTROL_BASE_PA + 0x20)
#define SYS_CTRL_CKEN_CTRL              (SYS_CONTROL_BASE_PA + 0x24)
#define SYS_CTRL_RSTEN_CTRL             (SYS_CONTROL_BASE_PA + 0x28)
#define SYS_CTRL_CKEN_SET_CTRL          (SYS_CONTROL_BASE_PA + 0x2C)
#define SYS_CTRL_CKEN_CLR_CTRL          (SYS_CONTROL_BASE_PA + 0x30)
#define SYS_CTRL_RSTEN_SET_CTRL         (SYS_CONTROL_BASE_PA + 0x34)
#define SYS_CTRL_RSTEN_CLR_CTRL         (SYS_CONTROL_BASE_PA + 0x38)
#define SYS_CTRL_USBHSMPH_CTRL          (SYS_CONTROL_BASE_PA + 0x40)
#define SYS_CTRL_USBHSMPH_STAT          (SYS_CONTROL_BASE_PA + 0x44)
#define SYS_CTRL_PLLSYS_CTRL            (SYS_CONTROL_BASE_PA + 0x48)
#define SYS_CTRL_SEMA_STAT              (SYS_CONTROL_BASE_PA + 0x4C)
#define SYS_CTRL_SEMA_SET_CTRL          (SYS_CONTROL_BASE_PA + 0x50)
#define SYS_CTRL_SEMA_CLR_CTRL          (SYS_CONTROL_BASE_PA + 0x54)
#define SYS_CTRL_SEMA_MASKA_CTRL        (SYS_CONTROL_BASE_PA + 0x58)
#define SYS_CTRL_SEMA_MASKB_CTRL        (SYS_CONTROL_BASE_PA + 0x5C)
#define SYS_CTRL_SEMA_MASKC_CTRL        (SYS_CONTROL_BASE_PA + 0x60)
#define SYS_CTRL_CKCTRL_CTRL            (SYS_CONTROL_BASE_PA + 0x64)
#define SYS_CTRL_COPRO_CTRL             (SYS_CONTROL_BASE_PA + 0x68)
#define SYS_CTRL_PLLSYS_KEY_CTRL        (SYS_CONTROL_BASE_PA + 0x6C)
#define SYS_CTRL_GMACA_CTRL             (SYS_CONTROL_BASE_PA + 0x78)
#define SYS_CTRL_GMAC_CTRL              (SYS_CONTROL_BASE_PA + 0x78)
#define SYS_CTRL_USBHSPHY_CTRL          (SYS_CONTROL_BASE_PA + 0x84)
#define SYS_CTRL_REF300_DIV             (SYS_CONTROL_BASE_PA + 0xF8)
#define SYS_CTRL_USB_CTRL               (SYS_CONTROL_BASE_PA + 0x90)
#define SYS_CTRL_UART_CTRL              (SYS_CONTROL_BASE_PA + 0x94)
#define SYS_CTRL_GMACB_CTRL             (SYS_CONTROL_BASE_PA + 0xEC)
#define SYS_CTRL_GMACA_DELAY_CTRL       (SYS_CONTROL_BASE_PA +0x100)
#define SYS_CTRL_GMACB_DELAY_CTRL       (SYS_CONTROL_BASE_PA +0x104)
#define SYS_CTRL_HCSL_CTRL              (SYS_CONTROL_BASE_PA +0x114)
#define SYS_CTRL_PCIEA_CTRL       		(SYS_CONTROL_BASE_PA +0x120)
#define SYS_CTRL_PCIEB_CTRL       		(SYS_CONTROL_BASE_PA +0x124)
#define SYS_CTRL_PCIEPHY_CTRL       	(SYS_CONTROL_BASE_PA +0x128)
#define SYS_CTRL_PCIEPHY_CR       		(SYS_CONTROL_BASE_PA +0x12C)
#define SYS_CTRL_PCIEA_POM0_MEM_ADDR	(SYS_CONTROL_BASE_PA +0x138)
#define SYS_CTRL_PCIEA_POM1_MEM_ADDR	(SYS_CONTROL_BASE_PA +0x13C)
#define SYS_CTRL_PCIEA_IN0_MEM_ADDR		(SYS_CONTROL_BASE_PA +0x140)
#define SYS_CTRL_PCIEA_IN1_MEM_ADDR		(SYS_CONTROL_BASE_PA +0x144)
#define SYS_CTRL_PCIEA_IN_IO_ADDR		(SYS_CONTROL_BASE_PA +0x148)
#define SYS_CTRL_PCIEA_IN_CFG0_ADDR		(SYS_CONTROL_BASE_PA +0x14C)
#define SYS_CTRL_PCIEA_IN_CFG1_ADDR		(SYS_CONTROL_BASE_PA +0x150)
#define SYS_CTRL_PCIEA_IN_MSG_ADDR		(SYS_CONTROL_BASE_PA +0x154)
#define SYS_CTRL_PCIEA_IN0_MEM_LIMIT	(SYS_CONTROL_BASE_PA +0x158)
#define SYS_CTRL_PCIEA_IN1_MEM_LIMIT	(SYS_CONTROL_BASE_PA +0x15C)
#define SYS_CTRL_PCIEA_IN_IO_LIMIT		(SYS_CONTROL_BASE_PA +0x160)
#define SYS_CTRL_PCIEA_IN_CFG0_LIMIT	(SYS_CONTROL_BASE_PA +0x164)
#define SYS_CTRL_PCIEA_IN_CFG1_LIMIT	(SYS_CONTROL_BASE_PA +0x168)
#define SYS_CTRL_PCIEA_IN_MSG_LIMIT		(SYS_CONTROL_BASE_PA +0x16C)
#define SYS_CTRL_PCIEA_AHB_SLAVE_CTRL	(SYS_CONTROL_BASE_PA +0x170)
#define SYS_CTRL_PCIEB_POM0_MEM_ADDR	(SYS_CONTROL_BASE_PA +0x174)
#define SYS_CTRL_PCIEB_POM1_MEM_ADDR	(SYS_CONTROL_BASE_PA +0x178)
#define SYS_CTRL_PCIEB_IN0_MEM_ADDR		(SYS_CONTROL_BASE_PA +0x17C)
#define SYS_CTRL_PCIEB_IN1_MEM_ADDR		(SYS_CONTROL_BASE_PA +0x180)
#define SYS_CTRL_PCIEB_IN_IO_ADDR		(SYS_CONTROL_BASE_PA +0x184)
#define SYS_CTRL_PCIEB_IN_CFG0_ADDR		(SYS_CONTROL_BASE_PA +0x188)
#define SYS_CTRL_PCIEB_IN_CFG1_ADDR		(SYS_CONTROL_BASE_PA +0x18C)
#define SYS_CTRL_PCIEB_IN_MSG_ADDR		(SYS_CONTROL_BASE_PA +0x190)
#define SYS_CTRL_PCIEB_IN0_MEM_LIMIT	(SYS_CONTROL_BASE_PA +0x194)
#define SYS_CTRL_PCIEB_IN1_MEM_LIMIT	(SYS_CONTROL_BASE_PA +0x198)
#define SYS_CTRL_PCIEB_IN_IO_LIMIT		(SYS_CONTROL_BASE_PA +0x19C)
#define SYS_CTRL_PCIEB_IN_CFG0_LIMIT	(SYS_CONTROL_BASE_PA +0x1A0)
#define SYS_CTRL_PCIEB_IN_CFG1_LIMIT	(SYS_CONTROL_BASE_PA +0x1A4)
#define SYS_CTRL_PCIEB_IN_MSG_LIMIT		(SYS_CONTROL_BASE_PA +0x1A8)
#define SYS_CTRL_PCIEB_AHB_SLAVE_CTRL	(SYS_CONTROL_BASE_PA +0x1AC)

/* System control multi-function pin function selection */
#define SYS_CTRL_SECONDARY_SEL        (SYS_CONTROL_BASE_PA + 0x14)
#define SYS_CTRL_TERTIARY_SEL         (SYS_CONTROL_BASE_PA + 0x8c)
#define SYS_CTRL_QUATERNARY_SEL       (SYS_CONTROL_BASE_PA + 0x94)
#define SYS_CTRL_DEBUG_SEL            (SYS_CONTROL_BASE_PA + 0x9c)
#define SYS_CTRL_ALTERNATIVE_SEL      (SYS_CONTROL_BASE_PA + 0xa4)
#define SYS_CTRL_PULLUP_SEL           (SYS_CONTROL_BASE_PA + 0xac)

/* Scratch registers */
#define SYS_CTRL_SCRATCHWORD0         (SYS_CONTROL_BASE_PA + 0xc4)
#define SYS_CTRL_SCRATCHWORD1         (SYS_CONTROL_BASE_PA + 0xc8)
#define SYS_CTRL_SCRATCHWORD2         (SYS_CONTROL_BASE_PA + 0xcc)
#define SYS_CTRL_SCRATCHWORD3         (SYS_CONTROL_BASE_PA + 0xd0)

#define SEC_CTRL_SECONDARY_SEL        (SEC_CONTROL_BASE_PA + 0x14)
#define SEC_CTRL_TERTIARY_SEL         (SEC_CONTROL_BASE_PA + 0x8c)
#define SEC_CTRL_QUATERNARY_SEL       (SEC_CONTROL_BASE_PA + 0x94)
#define SEC_CTRL_DEBUG_SEL            (SEC_CONTROL_BASE_PA + 0x9c)
#define SEC_CTRL_ALTERNATIVE_SEL      (SEC_CONTROL_BASE_PA + 0xa4)
#define SEC_CTRL_PULLUP_SEL           (SEC_CONTROL_BASE_PA + 0xac)

/* bit numbers of clock control register */
#define SYS_CTRL_CKEN_COPRO_BIT  0
#define SYS_CTRL_CKEN_DMA_BIT    1
#define SYS_CTRL_CKEN_CIPHER_BIT 2
#define SYS_CTRL_CKEN_SD_BIT     3
#define SYS_CTRL_CKEN_SATA_BIT   4
#define SYS_CTRL_CKEN_I2S_BIT    5
#define SYS_CTRL_CKEN_USBHS_BIT  6
#define SYS_CTRL_CKEN_MACA_BIT   7
#define SYS_CTRL_CKEN_MAC_BIT   SYS_CTRL_CKEN_MACA_BIT
#define SYS_CTRL_CKEN_PCIEA_BIT  8
#define SYS_CTRL_CKEN_STATIC_BIT 9
#define SYS_CTRL_CKEN_MACB_BIT   10
#define SYS_CTRL_CKEN_PCIEB_BIT  11
#define SYS_CTRL_CKEN_REF600_BIT 12
#define SYS_CTRL_CKEN_USBDEV_BIT 13
#define SYS_CTRL_CKEN_DDR_BIT    14
#define SYS_CTRL_CKEN_DDRPHY_BIT 15
#define SYS_CTRL_CKEN_DDRCK_BIT  16

/* bit numbers of reset control register */
#define SYS_CTRL_RSTEN_SCU_BIT          0
#define SYS_CTRL_RSTEN_COPRO_BIT        1
#define SYS_CTRL_RSTEN_ARM0_BIT         2
#define SYS_CTRL_RSTEN_ARM1_BIT         3
#define SYS_CTRL_RSTEN_USBHS_BIT        4
#define SYS_CTRL_RSTEN_USBHSPHYA_BIT    5
#define SYS_CTRL_RSTEN_USBPHYA_BIT      5
#define SYS_CTRL_RSTEN_MACA_BIT         6
#define SYS_CTRL_RSTEN_MAC_BIT	SYS_CTRL_RSTEN_MACA_BIT
#define SYS_CTRL_RSTEN_PCIEA_BIT        7
#define SYS_CTRL_RSTEN_SGDMA_BIT        8
#define SYS_CTRL_RSTEN_CIPHER_BIT       9
#define SYS_CTRL_RSTEN_DDR_BIT          10
#define SYS_CTRL_RSTEN_SATA_BIT         11
#define SYS_CTRL_RSTEN_SATA_LINK_BIT    12
#define SYS_CTRL_RSTEN_SATA_PHY_BIT     13
#define SYS_CTRL_RSTEN_PCIEPHY_BIT      14
#define SYS_CTRL_RSTEN_STATIC_BIT       15
#define SYS_CTRL_RSTEN_GPIO_BIT         16
#define SYS_CTRL_RSTEN_UART1_BIT        17
#define SYS_CTRL_RSTEN_UART2_BIT        18
#define SYS_CTRL_RSTEN_MISC_BIT         19
#define SYS_CTRL_RSTEN_I2S_BIT          20
#define SYS_CTRL_RSTEN_SD_BIT           21
#define SYS_CTRL_RSTEN_MACB_BIT         22
#define SYS_CTRL_RSTEN_PCIEB_BIT        23
#define SYS_CTRL_RSTEN_VIDEO_BIT        24
#define SYS_CTRL_RSTEN_DDR_PHY_BIT      25
#define SYS_CTRL_RSTEN_USBHSPHYB_BIT    26
#define SYS_CTRL_RSTEN_USBPHYB_BIT      26
#define SYS_CTRL_RSTEN_USBDEV_BIT       27
#define SYS_CTRL_RSTEN_ARMDBG_BIT       29
#define SYS_CTRL_RSTEN_PLLA_BIT         30
#define SYS_CTRL_RSTEN_PLLB_BIT         31

#define SYS_CTRL_USBHSMPH_IP_POL_A_BIT  0
#define SYS_CTRL_USBHSMPH_IP_POL_B_BIT  1
#define SYS_CTRL_USBHSMPH_IP_POL_C_BIT  2
#define SYS_CTRL_USBHSMPH_OP_POL_A_BIT  3
#define SYS_CTRL_USBHSMPH_OP_POL_B_BIT  4
#define SYS_CTRL_USBHSMPH_OP_POL_C_BIT  5

#define SYS_CTRL_GMAC_AUTOSPEED     3
#define SYS_CTRL_GMAC_RGMII         2
#define SYS_CTRL_GMAC_SIMPLE_MAX    1
#define SYS_CTRL_GMAC_CKEN_GTX      0

#define SYS_CTRL_CKCTRL_CTRL_ADDR     (SYS_CONTROL_BASE_PA + 0x64)

#define SYS_CTRL_CKCTRL_PCI_DIV_BIT 0
#define SYS_CTRL_CKCTRL_SLOW_BIT    8

#define SYS_CTRL_UART2_DEQ_EN       0
#define SYS_CTRL_UART3_DEQ_EN       1
#define SYS_CTRL_UART3_IQ_EN        2
#define SYS_CTRL_UART4_IQ_EN        3
#define SYS_CTRL_UART4_NOT_PCI_MODE 4

#define SYS_CTRL_USBHSPHY_SUSPENDM_MANUAL_ENABLE    16
#define SYS_CTRL_USBHSPHY_SUSPENDM_MANUAL_STATE     15
#define SYS_CTRL_USBHSPHY_ATE_ESET                  14
#define SYS_CTRL_USBHSPHY_TEST_DIN                   6
#define SYS_CTRL_USBHSPHY_TEST_ADD                   2
#define SYS_CTRL_USBHSPHY_TEST_DOUT_SEL              1
#define SYS_CTRL_USBHSPHY_TEST_CLK                   0

#define USB_REF_300_DIVIDER     8
#define USB_REF_300_INV         28
#define USB_REF_300_WCK_DUTY    29

#define PLLB_ENSAT              3
#define PLLB_OUTDIV             4
#define PLLB_REFDIV             8
#define USB_REF_600_DIVIDER     8

#define SYS_CTRL_USB_CTRL_USBAPHY_CKSEL_BIT	5
#define USB_CLK_XTAL0_XTAL1	0
#define USB_CLK_XTAL0		1
#define USB_CLK_INTERNAL	2
#define SYS_CTRL_USB_CTRL_USBAMUX_DEVICE_BIT	4
#define SYS_CTRL_USB_CTRL_USBPHY_REFCLKDIV_BIT	2
#define USB_PHY_REF_12MHZ	0
#define USB_PHY_REF_24MHZ	1
#define USB_PHY_REF_48MHZ	2
#define SYS_CTRL_USB_CTRL_USB_CKO_SEL_BIT	0
#define USB_INT_CLK_XTAL 	0
#define USB_INT_CLK_REF300	2
#define USB_INT_CLK_PLLB	3

#define SYS_CTRL_PCI_CTRL1_PCI_STATIC_RQ_BIT 11

/**
 * SATA related definitions
 */
#define ATA_PORT_CTL        0
#define ATA_PORT_FEATURE    1
#define ATA_PORT_NSECT      2
#define ATA_PORT_LBAL       3
#define ATA_PORT_LBAM       4
#define ATA_PORT_LBAH       5
#define ATA_PORT_DEVICE     6
#define ATA_PORT_COMMAND    7

#define SATA_0_REGS_BASE        (0x45900000)
#define SATA_1_REGS_BASE        (0x45910000)
#define SATA_DMA_REGS_BASE      (0x459a0000)
#define SATA_SGDMA_REGS_BASE    (0x459b0000)
#define SATA_HOST_REGS_BASE     (0x459e0000)

/* The offsets to the SATA registers */
#define SATA_ORB1_OFF           0
#define SATA_ORB2_OFF           1
#define SATA_ORB3_OFF           2
#define SATA_ORB4_OFF           3
#define SATA_ORB5_OFF           4

#define SATA_FIS_ACCESS         11
#define SATA_INT_STATUS_OFF     12  /* Read only */
#define SATA_INT_CLR_OFF        12  /* Write only */
#define SATA_INT_ENABLE_OFF     13  /* Read only */
#define SATA_INT_ENABLE_SET_OFF 13  /* Write only */
#define SATA_INT_ENABLE_CLR_OFF 14  /* Write only */
#define SATA_VERSION_OFF        15
#define SATA_CONTROL_OFF        23
#define SATA_COMMAND_OFF        24
#define SATA_PORT_CONTROL_OFF   25
#define SATA_DRIVE_CONTROL_OFF  26

/* The offsets to the link registers that are access in an asynchronous manner */
#define SATA_LINK_DATA     28
#define SATA_LINK_RD_ADDR  29
#define SATA_LINK_WR_ADDR  30
#define SATA_LINK_CONTROL  31

/* SATA interrupt status register fields */
#define SATA_INT_STATUS_EOC_RAW_BIT     ( 0 + 16) 
#define SATA_INT_STATUS_ERROR_BIT       ( 2 + 16)
#define SATA_INT_STATUS_EOADT_RAW_BIT   ( 1 + 16)

/* SATA core command register commands */
#define SATA_CMD_WRITE_TO_ORB_REGS              2
#define SATA_CMD_WRITE_TO_ORB_REGS_NO_COMMAND   4

#define SATA_CMD_BUSY_BIT 7

#define SATA_SCTL_CLR_ERR 0x00000316UL

#define SATA_OPCODE_MASK 0x3

#define SATA_LBAL_BIT    0
#define SATA_LBAM_BIT    8
#define SATA_LBAH_BIT    16
#define SATA_HOB_LBAH_BIT 24
#define SATA_DEVICE_BIT  24
#define SATA_NSECT_BIT   0
#define SATA_FEATURE_BIT 16
#define SATA_COMMAND_BIT 24
#define SATA_CTL_BIT     24

/* ATA status (7) register field definitions */
#define ATA_STATUS_BSY_BIT     7
#define ATA_STATUS_DRDY_BIT    6
#define ATA_STATUS_DF_BIT      5
#define ATA_STATUS_DRQ_BIT     3
#define ATA_STATUS_ERR_BIT     0

/* ATA device (6) register field definitions */
#define ATA_DEVICE_FIXED_MASK 0xA0
#define ATA_DEVICE_DRV_BIT 4
#define ATA_DEVICE_DRV_NUM_BITS 1
#define ATA_DEVICE_LBA_BIT 6

/* ATA control (0) register field definitions */
#define ATA_CTL_SRST_BIT   2

/* ATA Command register initiated commands */
#define ATA_CMD_INIT    0x91
#define ATA_CMD_IDENT   0xEC

#define SATA_STD_ASYNC_REGS_OFF 0x20
#define SATA_SCR_STATUS      0
#define SATA_SCR_ERROR       1
#define SATA_SCR_CONTROL     2
#define SATA_SCR_ACTIVE      3
#define SATA_SCR_NOTIFICAION 4

#define SATA_BURST_BUF_FORCE_EOT_BIT        0
#define SATA_BURST_BUF_DATA_INJ_ENABLE_BIT  1
#define SATA_BURST_BUF_DIR_BIT              2
#define SATA_BURST_BUF_DATA_INJ_END_BIT     3
#define SATA_BURST_BUF_FIFO_DIS_BIT         4
#define SATA_BURST_BUF_DIS_DREQ_BIT         5
#define SATA_BURST_BUF_DREQ_BIT             6

/* Button on GPIO 32 */
#define RECOVERY_BUTTON         (0x00000001 << 0)
#define RECOVERY_PRISEL_REG     SYS_CTRL_GPIO_PRIMSEL_CTRL_1
#define RECOVERY_SECSEL_REG     SYS_CTRL_GPIO_SECSEL_CTRL_1
#define RECOVERY_TERSEL_REG     SYS_CTRL_GPIO_TERTSEL_CTRL_1
#define RECOVERY_CLR_OE_REG     GPIO_2_CLR_OE
#define RECOVERY_DEBOUNCE_REG   GPIO_2_INPUT_DEBOUNCE_ENABLE
#define RECOVERY_DATA           GPIO_2_PA

/****
 * SD combo connected SPI flash memory.
 */
#define SD_BASE         (0x45400000)
#define SD_CONFIG 		(SD_BASE + 0x00)
#define SD_COMMAND      (SD_BASE + 0x04)  
#define SD_ARGUMENT     (SD_BASE + 0x08)
#define SD_DATA_SIZE    (SD_BASE + 0x0C)
#define SD_STATUS       (SD_BASE + 0x1C)
#define SD_RESPONSE1    (SD_BASE + 0x24)
#define SD_RESPONSE2    (SD_BASE + 0x28)

#define SD_FIFO_PORT    (SD_BASE + 0x40)
#define SD_FIFO_STATUS  (SD_BASE + 0x48)
#define SD_CLOCK_CTRL   (SD_BASE + 0x50)
#define SD_BANK0_CONFIG (SD_BASE + 0x60)

#define SD_NOR_ENABLE0 0x000001e0	/* only enable clk, dat in out and cs. */

#define SD_SPI_CONFIG  0x00018300 /* bank 0 in spi mode */
#define SD_SPI_PORT    0x00001408 /* bank 0 gap length 7 cycles */
#define SD_CLOCK_STOP  0x80000000
#define SD_CLOCK_RUN   0x0000000e /* 12.5MHz at 200MHz clock. */

/* command control data */
#define ID_DATA_SIZE   0x00010004
#define GET_ID1_CMD    0x904A0401   /* command 90 request 2 bytes returned */
#define GET_ID2_CMD    0x9F4A0401   /* command 9F request 3 bytes returned */
#define GET_ID3_CMD    0xAB4A0401   /* command AB request 2 bytes returned */
#define SD_CMD0        0x40010601
#define SD_CMD8		   0x48010601
#define SD_CMD55       0XB7010601
#define SD_ACMD41      0xA9010601
#define SD_CMD58       0xBA030601
#define SD_FLASH_READ  0x03400401
#define SD_READ_BLOCK  0x51410601

#define SD_IDLE_BIT    (1 <<  4)

/*
 * Software (bit-bang) SPI driver configuration
 */
#define SPI_CS          0x00000100      /*  spi device Chip select */
#define I2C_SCLK        0x00000020      /* PD 18: Shift clock */
#define I2C_MOSI        0x00000040      /* PD 17: Master Out, Slave In */
#define I2C_MISO        0x00000080      /* PD 16: Master In, Slave Out */

/* not required debounce input? \ */
                 /* port initialization needed */
#define  SPI_INIT       do { writel(SPI_CS, GPIO_1_SET);\
							 (*(u32 *)GPIO_1_OE) |= (SPI_CS | I2C_SCLK | I2C_MOSI);\
							 (*(u32 *)GPIO_1_PULL) |= (SPI_CS | I2C_SCLK | I2C_MOSI);\
							 (*(u32 *)GPIO_1_DRN) |= (SPI_CS | I2C_SCLK | I2C_MOSI);\
							 (*(u32 *)GPIO_1_DBC) &= ~(I2C_MISO);\
						} while (0)
						
#define SPI_READ        (((*(volatile unsigned int *)GPIO_1_PA) & I2C_MISO) == I2C_MISO ? 1 : 0)

#define SPI_SDA(bit)    do {      \
                        if(bit){ (*(volatile u32 *)GPIO_1_SET) = I2C_MOSI; }\
                        else   { (*(volatile u32 *)GPIO_1_CLR) = I2C_MOSI; }\
                        asm volatile ("" : : : "memory");\
                        } while (0)
#define SPI_SCL(bit)    do {                                            \
                        if(bit) (*(volatile u32 *)GPIO_1_SET) = I2C_SCLK; \
                        else    (*(volatile u32 *)GPIO_1_CLR) = I2C_SCLK; \
                        asm volatile ("" : : : "memory");\
                        } while (0)

#define SPI_DELAY       do { volatile int i; for ( i=1 ; i < 0; i--)  ; asm volatile ("" : : : "memory");\
 				} while (0)                 /* No delay is needed */


#define USBA_POWO_SEC_MFP  10
#define USBA_OVERI_SEC_MFP 11
#define USBA_POWO_TER_MFP  48
#define USBA_OVERI_TER_MFP 49

#define USBB_POWO_SEC_MFP  28
#define USBB_OVERI_SEC_MFP 29
#define USBB_POWO_TER_MFP  5
#define USBB_OVERI_TER_MFP 0

#endif
