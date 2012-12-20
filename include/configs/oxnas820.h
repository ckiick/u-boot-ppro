#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SKIP_LOWLEVEL_INIT	/* chain loading */

#undef CONFIG_USE_IRQ
#define CONFIG_SYS_VSNPRINTF
#define CONFIG_CMDLINE_TAG	/* needs it. not documented. */

/* We have an LED, lets try to use it. */
#define	CONFIG_STATUS_LED	// we have a status LED
#define CONFIG_BOARD_SPECIFIC_LED	// we implement led funcs
#define STATUS_LED_BIT	0x00020000	// mask for led in gpio register
#define STATUS_LED_STATE STATUS_LED_OFF	// start off off
#define STATUS_LED_PERIOD (CONFIG_SYS_HZ/10)	// blink period.
#define STATUS_LED_BOOT STATUS_LED_BIT	// use status led for boot status

#define OXNAS_USE_NAND 1
#define OXNAS_USE_NAND_ENV 1
#define OXNAS_USE_SATA 1
// #define OXNAS_USE_SATA_ENV 1
#if (OXNAS_USE_NAND_ENV && OXNAS_USE_SATA_ENV)
#error "Only 1 env location can be used."
#endif

/* define this to (try to) get USB working */
#define OXNAS_USE_USB 1

#define RPSCLK 6250000
#define CONFIG_SYS_HZ ((RPSCLK) / 16) 

#define CONFIG_SYS_TEXT_BASE 0x60c00000
#define CONFIG_STANDALONE_LOAD_ADDR 0x60000000
#define CONFIG_SYS_LOAD_ADDR 0x60500000
#define CONFIG_EXCEPTION_BASE 0x50000000
#define CONFIG_IDENT_STRING "OXNAS820"

/* Architecture */
#define CONFIG_ARM11MPC 1
#define CONFIG_OXNAS820 1

/*
 * Network
 */
#define CONFIG_ETHADDR      00:30:e0:00:00:01

/*
 * Serial
 */
#define CONFIG_BAUDRATE 115200
#define CONFIG_SYS_BAUDRATE_TABLE   { 9600, 19200, 38400, 57600, 115200 }

/*
 * Command line configuration
 */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_CONSOLE
/* already defined #define CONFIG_CMD_CRC32 */
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EDITENV
/* already defined #define CONFIG_CMD_GO */
#ifdef CONFIG_STATUS_LED
#define CONFIG_CMD_LED
#endif
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SOURCE
#ifdef OXNAS_USE_USB
#define CONFIG_CMD_USB
#endif
/* NAND memory support */
#ifdef OXNAS_USE_NAND
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_ECC		/* turn ecc on/off */
#endif
/* Disk (SATA) support */
#ifdef OXNAS_USE_SATA
#define CONFIG_CMD_SATA
#define CONFIG_CMD_EXT2
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_SATA_MAX_DEVICE 2
#endif

/*
 * BOOTP
 */
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * Network
 */
#define CONFIG_NETMASK      255.255.255.0
#define CONFIG_IPADDR       192.168.1.75
#define CONFIG_SERVERIP     192.168.1.66
#define CONFIG_SYS_AUTOLOAD "n"
#define CONFIG_NETCONSOLE

/*
 * Console
 */
#define CONFIG_SYS_CBSIZE 256
#define CONFIG_SYS_PROMPT ">> "
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/*
 * Stack
 */
#define CONFIG_STACKSIZE (128 * 1024)
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4 * 1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4 * 1024)	/* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS 1
#define PHYS_SDRAM_1 0x60000000
#define PHYS_SDRAM_1_SIZE (128 * 1024 * 1024)

/*
 * Environment
 */
#define COMMON_BOOTARGS " console=ttyS0,115200 elevator=cfq mac_adr=0x00,0x30,0xe0,0x00,0x00,0x01 mem=128M poweroutage=yes"

#if OXNAS_USE_NAND_ENV
#define CONFIG_ENV_SIZE (128*1024)	/* NAND eraseblock size (for now) */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET 0xa0000
#define CONFIG_ENV_RANGE (3 * CONFIG_ENV_SIZE)	/* allow 2 backup blocks */
#define CONFIG_SYS_NAND_ENV_ECC_ON	/* ECC is always on for env block */
#define CONFIG_BOOTARGS	"root=/dev/sda1 ubi.mtd=2,512" COMMON_BOOTARGS
#define CONFIG_BOOTCOMMAND	"run boot_custom"
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"load_custom_nand=nboot 60500000 0 500000\0" \
	"load_custom_nand2=nboot 60500000 0 B00000\0" \
	"boot_custom=run load_custom_nand boot || run load_custom_nand2 boot\0"

#elif OXNAS_USE_SATA_ENV
#define CONFIG_ENV_SIZE (8*1024)
#define CONFIG_ENV_IS_IN_DISK
#define CONFIG_SYS_DISK_ENV_INTERFACE	"sata"
#define CONFIG_SYS_DISK_ENV_DEV	0
/* these are determined by stage1 loader, which is why all the constants. */
/* offsets in bytes from start of device */
#define CONFIG_ENV_OFFSET ((64 * 512) + (256 * 1024) - CONFIG_ENV_SIZE - 1024)
#define CONFIG_ENV_OFFSET_REDUND (((8 + 64) * 512) + (256 * 1024))
#define CONFIG_BOOTARGS	"root=/dev/sda2" COMMON_BOOTARGS
#define CONFIG_BOOTCOMMAND "run dload1 bootm || run dload2 bootm || lightled"
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"dload1=sataboot 60500000 0:1\0"	\
	"dload2=sataboot 60500000 0:2\0"	\
	"lightled=ledfail 1\0"
#else /* no env? */
#define CONFIG_ENV_IS_NOWHERE
#endif	/* OXNAS_USE_*_ENV */

#define CONFIG_BOOTDELAY 2	/* default is short. */
#define CONFIG_BOOTFILE		"uImage" /* file to load */

/*
 * NAND
 */
#ifdef OXNAS_USE_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_NAND_BASE 0x41000000
#define	CONFIG_SYS_NAND_ADDRESS_LATCH	(CONFIG_SYS_NAND_BASE + (1<<18))
#define	CONFIG_SYS_NAND_COMMAND_LATCH	(CONFIG_SYS_NAND_BASE + (1<<19))
#endif

/* don't know if we need or want these */
//#define CONFIG_MTD_DEVICE
//#define CONFIG_MTD_PARTITIONS

/*
 * Flash
 */
#define CONFIG_SYS_NO_FLASH

/*
 * Disk support
 */
#if OXNAS_USE_SATA
#define CONFIG_SATA
#define CONFIG_LBA48
#define CONFIG_SYS_64BIT_LBA
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_SATA_MAX_DEVICE 2
#endif

/*
 * SDRAM
 */
#define CONFIG_SYS_SDRAM_BASE (0x60000000)
#define CONFIG_MAX_RAM_BANK_SIZE (128 * 1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN (1024 * 1024)
#define CONFIG_SYS_INIT_SP_ADDR (0x60000000 + 128 * 1024 * 1024 - 64 * 1024)
//#define CONFIG_SYS_GBL_DATA_SIZE	128 // size (bytes) for initial data
// #define CONFIG_SYS_GBL_DATA_OFFSET	(128 * 1024 * 1024 - 64 * 1024 + 256)
// #define CONFIG_SYS_INIT_SP_ADDR (CONFIG_SYS_TEXT_BASE - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_GBL_DATA_SIZE - 12)
/* XXX Fix this. We should not need to define this. */
// #define CONFIG_SYS_GBL_DATA_OFFSET	(-(CONFIG_SYS_MALLOC_LEN + CONFIG_SYS_GBL_DATA_SIZE))

#define CONFIG_SYS_MEMTEST_START 0x60000000
#define CONFIG_SYS_MEMTEST_END ((128 * 1024 * 1024 ) + CONFIG_SYS_MEMTEST_START - 1)

/*
 * UART
 */
#define CONFIG_SYS_NS16550 1
#define CONFIG_SYS_NS16550_SERIAL 1
#define CONFIG_SYS_NS16550_COM1 (0x44200000)
#define CONFIG_SYS_NS16550_REG_SIZE 1
#define CONFIG_CONS_INDEX 1
#define CONFIG_SYS_NS16550_CLK (RPSCLK)

/*
 * USB Host. USB needs work.
 */
#ifdef OXNAS_USE_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_OXNAS820
#define CONFIG_EHCI_IS_TDI
#endif

/*
 * Shell
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2 "-> "
#define CONFIG_SYS_LONGHELP

#endif
