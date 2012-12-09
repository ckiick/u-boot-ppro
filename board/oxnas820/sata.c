#include <common.h>
#include <asm/arch-oxnas820/regs.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <ide.h>
#include <ata.h>
#include <libata.h>

#ifdef CONFIG_SATA

#define ide_led(x, y) {}

extern block_dev_desc_t sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];


#define SATA_DMA_CHANNEL 0
#define IDE_TIME_OUT	2000	/* 2 sec timeout */
#define IDE_SPIN_UP_TIME_OUT 5000 /* 5 sec spin-up timeout */

#define CONFIG_SYS_ATA_REG_OFFSET 0

#define DMA_CTRL_STATUS      (0x0)
#define DMA_BASE_SRC_ADR     (0x4)
#define DMA_BASE_DST_ADR     (0x8)
#define DMA_BYTE_CNT         (0xC)
#define DMA_CURRENT_SRC_ADR  (0x10)
#define DMA_CURRENT_DST_ADR  (0x14)
#define DMA_CURRENT_BYTE_CNT (0x18)
#define DMA_INTR_ID          (0x1C)
#define DMA_INTR_CLEAR_REG   (DMA_CURRENT_SRC_ADR)

#define DMA_CALC_REG_ADR(channel, register) ((volatile u32*)(DMA_BASE_PA + ((channel) << 5) + (register)))

#define DMA_CTRL_STATUS_FAIR_SHARE_ARB            (1 << 0)
#define DMA_CTRL_STATUS_IN_PROGRESS               (1 << 1)
#define DMA_CTRL_STATUS_SRC_DREQ_MASK             (0x0000003C)
#define DMA_CTRL_STATUS_SRC_DREQ_SHIFT            (2)
#define DMA_CTRL_STATUS_DEST_DREQ_MASK            (0x000003C0)
#define DMA_CTRL_STATUS_DEST_DREQ_SHIFT           (6)
#define DMA_CTRL_STATUS_INTR                      (1 << 10)
#define DMA_CTRL_STATUS_NXT_FREE                  (1 << 11)
#define DMA_CTRL_STATUS_RESET                     (1 << 12)
#define DMA_CTRL_STATUS_DIR_MASK                  (0x00006000)
#define DMA_CTRL_STATUS_DIR_SHIFT                 (13)
#define DMA_CTRL_STATUS_SRC_ADR_MODE              (1 << 15)
#define DMA_CTRL_STATUS_DEST_ADR_MODE             (1 << 16)
#define DMA_CTRL_STATUS_TRANSFER_MODE_A           (1 << 17)
#define DMA_CTRL_STATUS_TRANSFER_MODE_B           (1 << 18)
#define DMA_CTRL_STATUS_SRC_WIDTH_MASK            (0x00380000)
#define DMA_CTRL_STATUS_SRC_WIDTH_SHIFT           (19)
#define DMA_CTRL_STATUS_DEST_WIDTH_MASK           (0x01C00000)
#define DMA_CTRL_STATUS_DEST_WIDTH_SHIFT          (22)
#define DMA_CTRL_STATUS_PAUSE                     (1 << 25)
#define DMA_CTRL_STATUS_INTERRUPT_ENABLE          (1 << 26)
#define DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED      (1 << 27)
#define DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED (1 << 28)
#define DMA_CTRL_STATUS_STARVE_LOW_PRIORITY       (1 << 29)
#define DMA_CTRL_STATUS_INTR_CLEAR_ENABLE         (1 << 30)

#define DMA_BYTE_CNT_MASK        ((1 << 21) - 1)
#define DMA_BYTE_CNT_WR_EOT_MASK (1 << 30)
#define DMA_BYTE_CNT_RD_EOT_MASK (1 << 31)
#define DMA_BYTE_CNT_BURST_MASK  (1 << 28)

#define MAKE_FIELD(value, num_bits, bit_num) (((value) & ((1 << (num_bits)) - 1)) << (bit_num))

typedef enum oxnas_dma_mode {
    OXNAS_DMA_MODE_FIXED,
    OXNAS_DMA_MODE_INC
} oxnas_dma_mode_t;

typedef enum oxnas_dma_direction {
    OXNAS_DMA_TO_DEVICE,
    OXNAS_DMA_FROM_DEVICE
} oxnas_dma_direction_t;

/* The available buses to which the DMA controller is attached */
typedef enum oxnas_dma_transfer_bus
{
    OXNAS_DMA_SIDE_A,
    OXNAS_DMA_SIDE_B
} oxnas_dma_transfer_bus_t;

/* Direction of data flow between the DMA controller's pair of interfaces */
typedef enum oxnas_dma_transfer_direction
{
    OXNAS_DMA_A_TO_A,
    OXNAS_DMA_B_TO_A,
    OXNAS_DMA_A_TO_B,
    OXNAS_DMA_B_TO_B
} oxnas_dma_transfer_direction_t;

/* The available data widths */
typedef enum oxnas_dma_transfer_width
{
    OXNAS_DMA_TRANSFER_WIDTH_8BITS,
    OXNAS_DMA_TRANSFER_WIDTH_16BITS,
    OXNAS_DMA_TRANSFER_WIDTH_32BITS
} oxnas_dma_transfer_width_t;

/* The mode of the DMA transfer */
typedef enum oxnas_dma_transfer_mode
{
    OXNAS_DMA_TRANSFER_MODE_SINGLE,
    OXNAS_DMA_TRANSFER_MODE_BURST
} oxnas_dma_transfer_mode_t;

/* The available transfer targets */
typedef enum oxnas_dma_dreq
{
    OXNAS_DMA_DREQ_SATA   = 0,
    OXNAS_DMA_DREQ_MEMORY = 15
} oxnas_dma_dreq_t;

typedef struct oxnas_dma_device_settings {
    unsigned long address_;
    unsigned      fifo_size_;   // Chained transfers must take account of FIFO offset at end of previous transfer
    unsigned char dreq_;
    unsigned      read_eot_:1;
    unsigned      read_final_eot_:1;
    unsigned      write_eot_:1;
    unsigned      write_final_eot_:1;
    unsigned      bus_:1;
    unsigned      width_:2;
    unsigned      transfer_mode_:1;
    unsigned      address_mode_:1;
    unsigned      address_really_fixed_:1;
} oxnas_dma_device_settings_t;

static const int MAX_NO_ERROR_LOOPS  = 100000;  /* 1 second in units of 10uS */
static const int MAX_DMA_XFER_LOOPS  = 300000;  /* 30 seconds in units of 100uS */
static const int MAX_DMA_ABORT_LOOPS = 10000;   /* 0.1 second in units of 10uS */
static const int MAX_SRC_READ_LOOPS  = 10000;   /* 0.1 second in units of 10uS */
static const int MAX_SRC_WRITE_LOOPS = 10000;   /* 0.1 second in units of 10uS */
static const int MAX_NOT_BUSY_LOOPS  = 10000;   /* 1 second in units of 100uS */

/* The internal SATA drive on which we should attempt to find partitions */
static volatile u32* sata_regs_base[2] = 
{
    (volatile u32*)SATA_0_REGS_BASE,
    (volatile u32*)SATA_1_REGS_BASE,
    
};
static u32 wr_sata_orb1[2] = { 0, 0 };
static u32 wr_sata_orb2[2] = { 0, 0 };
static u32 wr_sata_orb3[2] = { 0, 0 };
static u32 wr_sata_orb4[2] = { 0, 0 };

static oxnas_dma_device_settings_t oxnas_sata_dma_settings = {
    .address_              = SATA_DATA_BASE_PA,
    .fifo_size_            = 16,
    .dreq_                 = OXNAS_DMA_DREQ_SATA,
    .read_eot_             = 0,
    .read_final_eot_       = 1,
    .write_eot_            = 0,
    .write_final_eot_      = 1,
    .bus_                  = OXNAS_DMA_SIDE_B,
    .width_                = OXNAS_DMA_TRANSFER_WIDTH_32BITS,
    .transfer_mode_        = OXNAS_DMA_TRANSFER_MODE_BURST,
    .address_mode_         = OXNAS_DMA_MODE_FIXED,
    .address_really_fixed_ = 0
};

oxnas_dma_device_settings_t oxnas_ram_dma_settings = {
    .address_              = 0,
    .fifo_size_            = 0,
    .dreq_                 = OXNAS_DMA_DREQ_MEMORY,
    .read_eot_             = 1,
    .read_final_eot_       = 1,
    .write_eot_            = 1,
    .write_final_eot_      = 1,
    .bus_                  = OXNAS_DMA_SIDE_A,
    .width_                = OXNAS_DMA_TRANSFER_WIDTH_32BITS,
    .transfer_mode_        = OXNAS_DMA_TRANSFER_MODE_BURST,
    .address_mode_         = OXNAS_DMA_MODE_FIXED,
    .address_really_fixed_ = 1
};

static void xfer_wr_shadow_to_orbs(int device)
{
    *(sata_regs_base[device] + SATA_ORB1_OFF) = wr_sata_orb1[device];
    *(sata_regs_base[device] + SATA_ORB2_OFF) = wr_sata_orb2[device];
    *(sata_regs_base[device] + SATA_ORB3_OFF) = wr_sata_orb3[device];
    *(sata_regs_base[device] + SATA_ORB4_OFF) = wr_sata_orb4[device];
}

static inline void device_select(int device)
{
    /* master/slave has no meaning to SATA core */
}

static int disk_present[CONFIG_SYS_SATA_MAX_DEVICE];

unsigned char oxnas_sata_inb(int device, int port)
{
    unsigned char val = 0;

    /* Only permit accesses to disks found to be present during ide_preinit() */
    if (!disk_present[device]) {
        return ATA_STAT_FAULT;
    }

    device_select(device);

    switch (port) {
        case ATA_PORT_CTL:
            val = (*(sata_regs_base[device] + SATA_ORB4_OFF) & (0xFFUL << SATA_CTL_BIT)) >> SATA_CTL_BIT;
            break;
        case ATA_PORT_FEATURE:
            val = (*(sata_regs_base[device] + SATA_ORB2_OFF) & (0xFFUL << SATA_FEATURE_BIT)) >> SATA_FEATURE_BIT;
            break;
        case ATA_PORT_NSECT:
            val = (*(sata_regs_base[device] + SATA_ORB2_OFF) & (0xFFUL << SATA_NSECT_BIT)) >> SATA_NSECT_BIT;
            break;
        case ATA_PORT_LBAL:
            val = (*(sata_regs_base[device] + SATA_ORB3_OFF) & (0xFFUL << SATA_LBAL_BIT)) >> SATA_LBAL_BIT;
            break;
        case ATA_PORT_LBAM:
            val = (*(sata_regs_base[device] + SATA_ORB3_OFF) & (0xFFUL << SATA_LBAM_BIT)) >> SATA_LBAM_BIT;
            break;
        case ATA_PORT_LBAH:
            val = (*(sata_regs_base[device] + SATA_ORB3_OFF) & (0xFFUL << SATA_LBAH_BIT)) >> SATA_LBAH_BIT;
            break;
        case ATA_PORT_DEVICE:
            val = (*(sata_regs_base[device] + SATA_ORB3_OFF) & (0xFFUL << SATA_HOB_LBAH_BIT)) >> SATA_HOB_LBAH_BIT;
            val |= (*(sata_regs_base[device] + SATA_ORB1_OFF) & (0xFFUL << SATA_DEVICE_BIT)) >> SATA_DEVICE_BIT;
            break;
        case ATA_PORT_COMMAND:
            val = (*(sata_regs_base[device] + SATA_ORB2_OFF) & (0xFFUL << SATA_COMMAND_BIT)) >> SATA_COMMAND_BIT;
            val |= ATA_STAT_DRQ ;
            break;
        default:
            printf("ide_inb() Unknown port = %d\n", port);
            break;
    }

//    printf("inb: %d:%01x => %02x\n", device, port, val);

    return val;
}

/**
 * Possible that ATA status will not become no-error, so must have timeout
 * @returns An int which is zero on error
 */
static inline int wait_no_error(int device)
{
    int status = 0;

	/* Check for ATA core error */
	if (*(sata_regs_base[device] + SATA_INT_STATUS_OFF) & (1 << SATA_INT_STATUS_ERROR_BIT)) {
		printf("wait_no_error() SATA core flagged error\n");
	} else {
		int loops = MAX_NO_ERROR_LOOPS;
		do {
			/* Check for ATA device error */
			if (!(oxnas_sata_inb(device, ATA_PORT_COMMAND) & (1 << ATA_STATUS_ERR_BIT))) {
				status = 1;
				break;
			}
			udelay(10);
		} while (--loops);

		if (!loops) {
			printf("wait_no_error() Timed out of wait for SATA no-error condition\n");
		}
	}

    return status;
}

/**
 * Expect SATA command to always finish, perhaps with error
 * @returns An int which is zero on error
 */
static inline int wait_sata_command_not_busy(int device)
{
    /* Wait for data to be available */
    int status = 0;
    int loops = MAX_NOT_BUSY_LOOPS;
    do {
        if (!(*(sata_regs_base[device] + SATA_COMMAND_OFF) & (1 << SATA_CMD_BUSY_BIT) )) {
            status = 1;
            break;
        }
        udelay(100);
    } while (--loops);

    if (!loops) {
        printf("wait_sata_command_not_busy() Timed out of wait for SATA command to finish\n");
    }

    return status;
}

void oxnas_sata_outb(int device, int port, unsigned char val)
{
    typedef enum send_method {
        SEND_NONE,
        SEND_SIMPLE,
        SEND_CMD,
        SEND_CTL,
    } send_method_t;

    /* Only permit accesses to disks found to be present during ide_preinit() */
    if (!disk_present[device]) {
        return;
    }
    
//    printf("outb: %d:%01x <= %02x\n", device, port, val);

    device_select(device);

    send_method_t send_regs = SEND_NONE;
    switch (port) {
        case ATA_PORT_CTL:
            wr_sata_orb4[device] &= ~(0xFFUL << SATA_CTL_BIT);
            wr_sata_orb4[device] |= (val << SATA_CTL_BIT);
            send_regs = SEND_CTL;
            break;
        case ATA_PORT_FEATURE:
            wr_sata_orb2[device] &= ~(0xFFUL << SATA_FEATURE_BIT);
            wr_sata_orb2[device] |= (val << SATA_FEATURE_BIT);
            send_regs = SEND_SIMPLE;
            break;
        case ATA_PORT_NSECT:
            wr_sata_orb2[device] &= ~(0xFFUL << SATA_NSECT_BIT);
            wr_sata_orb2[device] |= (val << SATA_NSECT_BIT);
            send_regs = SEND_SIMPLE;
            break;
        case ATA_PORT_LBAL:
            wr_sata_orb3[device] &= ~(0xFFUL << SATA_LBAL_BIT);
            wr_sata_orb3[device] |= (val << SATA_LBAL_BIT);
            send_regs = SEND_SIMPLE;
            break;
        case ATA_PORT_LBAM:
            wr_sata_orb3[device] &= ~(0xFFUL << SATA_LBAM_BIT);
            wr_sata_orb3[device] |= (val << SATA_LBAM_BIT);
            send_regs = SEND_SIMPLE;
            break;
        case ATA_PORT_LBAH:
            wr_sata_orb3[device] &= ~(0xFFUL << SATA_LBAH_BIT);
            wr_sata_orb3[device] |= (val << SATA_LBAH_BIT);
            send_regs = SEND_SIMPLE;
            break;
        case ATA_PORT_DEVICE:
            wr_sata_orb1[device] &= ~(0xFFUL << SATA_DEVICE_BIT);
            wr_sata_orb1[device] |= ((val & 0xf0) << SATA_DEVICE_BIT);
            wr_sata_orb3[device] &= ~(0xFFUL << SATA_HOB_LBAH_BIT);
            wr_sata_orb3[device] |= ((val & 0x0f) << SATA_HOB_LBAH_BIT);
            send_regs = SEND_SIMPLE;
            break;
        case ATA_PORT_COMMAND:
            wr_sata_orb2[device] &= ~(0xFFUL << SATA_COMMAND_BIT);
            wr_sata_orb2[device] |= (val << SATA_COMMAND_BIT);
            send_regs = SEND_CMD;
            break;
        default:
            printf("ide_outb() Unknown port = %d\n", port);
    }

    u32 command;
    switch (send_regs) {
        case SEND_CMD:
            wait_sata_command_not_busy(device);
            command = *(sata_regs_base[device] + SATA_COMMAND_OFF);
            command &= ~SATA_OPCODE_MASK;
            command |= SATA_CMD_WRITE_TO_ORB_REGS;
            xfer_wr_shadow_to_orbs(device);
            wait_sata_command_not_busy(device);
            *(sata_regs_base[device] + SATA_COMMAND_OFF) = command;
            if (!wait_no_error(device)) {
                printf("oxnas_sata_outb() Wait for ATA no-error timed-out\n");
            }
            break;
        case SEND_CTL:
            wait_sata_command_not_busy(device);
            command = *(sata_regs_base[device] + SATA_COMMAND_OFF);
            command &= ~SATA_OPCODE_MASK;
            command |= SATA_CMD_WRITE_TO_ORB_REGS_NO_COMMAND;
            xfer_wr_shadow_to_orbs(device);
            wait_sata_command_not_busy(device);
            *(sata_regs_base[device] + SATA_COMMAND_OFF) = command;
            if (!wait_no_error(device)) {
                printf("oxnas_sata_outb() Wait for ATA no-error timed-out\n");
            }
            break;
        default:
            break;
    }
}

static u32 encode_start(u32 ctrl_status)
{
    return ctrl_status & ~DMA_CTRL_STATUS_PAUSE;
}

/* start a paused DMA transfer in channel 0 of the SATA DMA core */
static void dma_start(void)
{
    unsigned int reg;
    reg = readl(SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
    reg = encode_start(reg);
    writel( reg, SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
}

static unsigned long encode_control_status(
    oxnas_dma_device_settings_t* src_settings,
    oxnas_dma_device_settings_t* dst_settings)
{
    unsigned long ctrl_status;
    oxnas_dma_transfer_direction_t direction;

    ctrl_status  = DMA_CTRL_STATUS_PAUSE;                                       // Paused
    ctrl_status |= DMA_CTRL_STATUS_FAIR_SHARE_ARB;                              // High priority
    ctrl_status |= (src_settings->dreq_ << DMA_CTRL_STATUS_SRC_DREQ_SHIFT);     // Dreq
    ctrl_status |= (dst_settings->dreq_ << DMA_CTRL_STATUS_DEST_DREQ_SHIFT);    // Dreq
    ctrl_status &= ~DMA_CTRL_STATUS_RESET;                                      // !RESET

    // Use new interrupt clearing register
    ctrl_status |= DMA_CTRL_STATUS_INTR_CLEAR_ENABLE;

    // Setup the transfer direction and burst/single mode for the two DMA busses
    if (src_settings->bus_ == OXNAS_DMA_SIDE_A) {
        // Set the burst/single mode for bus A based on src device's settings
        if (src_settings->transfer_mode_ == OXNAS_DMA_TRANSFER_MODE_BURST) {
            ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_A;
        } else {
            ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_A;
        }

        if (dst_settings->bus_ == OXNAS_DMA_SIDE_A) {
            direction = OXNAS_DMA_A_TO_A;
        } else {
            direction = OXNAS_DMA_A_TO_B;

            // Set the burst/single mode for bus B based on dst device's settings
            if (dst_settings->transfer_mode_ == OXNAS_DMA_TRANSFER_MODE_BURST) {
                ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_B;
            } else {
                ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_B;
            }
        }
    } else {
        // Set the burst/single mode for bus B based on src device's settings
        if (src_settings->transfer_mode_ == OXNAS_DMA_TRANSFER_MODE_BURST) {
            ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_B;
        } else {
            ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_B;
        }

        if (dst_settings->bus_ == OXNAS_DMA_SIDE_A) {
            direction = OXNAS_DMA_B_TO_A;
            
            // Set the burst/single mode for bus A based on dst device's settings
            if (dst_settings->transfer_mode_ == OXNAS_DMA_TRANSFER_MODE_BURST) {
                ctrl_status |= DMA_CTRL_STATUS_TRANSFER_MODE_A;
            } else {
                ctrl_status &= ~DMA_CTRL_STATUS_TRANSFER_MODE_A;
            }
        } else {
            direction = OXNAS_DMA_B_TO_B;
        }
    }
    ctrl_status |= (direction << DMA_CTRL_STATUS_DIR_SHIFT);

    // Setup source address mode fixed or increment
    if (src_settings->address_mode_ == OXNAS_DMA_MODE_FIXED) {
        // Fixed address
        ctrl_status &= ~(DMA_CTRL_STATUS_SRC_ADR_MODE);
        
        // Set up whether fixed address is _really_ fixed
        if (src_settings->address_really_fixed_) {
            ctrl_status |= DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED;
        } else {
            ctrl_status &= ~DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED;
        }        
    } else {
        // Incrementing address
        ctrl_status |= DMA_CTRL_STATUS_SRC_ADR_MODE;
        ctrl_status &= ~DMA_CTRL_STATUS_SOURCE_ADDRESS_FIXED;
    }

    // Setup destination address mode fixed or increment
    if (dst_settings->address_mode_ == OXNAS_DMA_MODE_FIXED) {
        // Fixed address
        ctrl_status &= ~(DMA_CTRL_STATUS_DEST_ADR_MODE);
        
        // Set up whether fixed address is _really_ fixed
        if (dst_settings->address_really_fixed_) {
            ctrl_status |= DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED;
        } else {
            ctrl_status &= ~DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED;
        }
    } else {
        // Incrementing address
        ctrl_status |= DMA_CTRL_STATUS_DEST_ADR_MODE;
        ctrl_status &= ~DMA_CTRL_STATUS_DESTINATION_ADDRESS_FIXED;
    }

    // Set up the width of the transfers on the DMA buses
    ctrl_status |= (src_settings->width_ << DMA_CTRL_STATUS_SRC_WIDTH_SHIFT);
    ctrl_status |= (dst_settings->width_ << DMA_CTRL_STATUS_DEST_WIDTH_SHIFT);

    // Setup the priority arbitration scheme
    ctrl_status &= ~DMA_CTRL_STATUS_STARVE_LOW_PRIORITY;    // !Starve low priority

    return ctrl_status;
}

static u32 encode_final_eot(
    oxnas_dma_device_settings_t* src_settings,
    oxnas_dma_device_settings_t* dst_settings,
    unsigned long length)
{
    // Write the length, with EOT configuration for a final transfer
    unsigned long encoded = length;
    if (dst_settings->write_final_eot_) {
        encoded |= DMA_BYTE_CNT_WR_EOT_MASK;
    } else {
        encoded &= ~DMA_BYTE_CNT_WR_EOT_MASK;
    }
    if (src_settings->read_final_eot_) {
        encoded |= DMA_BYTE_CNT_RD_EOT_MASK;
    } else {
        encoded &= ~DMA_BYTE_CNT_RD_EOT_MASK;
    }
/*    if((src_settings->transfer_mode_) ||
       (src_settings->transfer_mode_)) {
        encoded |= DMA_BYTE_CNT_BURST_MASK;
    } else {
        encoded &= ~DMA_BYTE_CNT_BURST_MASK;
    }*/
    return encoded;
}

static void dma_start_write(ulong* buffer, int num_bytes)
{
    // Assemble complete memory settings
    oxnas_dma_device_settings_t mem_settings = oxnas_ram_dma_settings;
    mem_settings.address_ = (unsigned long)buffer;
    mem_settings.address_mode_ = OXNAS_DMA_MODE_INC;

    writel(encode_control_status(&mem_settings, &oxnas_sata_dma_settings),
        SATA_DMA_REGS_BASE + DMA_CTRL_STATUS);
    writel(mem_settings.address_, SATA_DMA_REGS_BASE + DMA_BASE_SRC_ADR);
    writel(oxnas_sata_dma_settings.address_, SATA_DMA_REGS_BASE + DMA_BASE_DST_ADR);
    writel(encode_final_eot(&mem_settings, &oxnas_sata_dma_settings, num_bytes),
        SATA_DMA_REGS_BASE + DMA_BYTE_CNT);
             
    dma_start();
}            

static void dma_start_read(ulong* buffer, int num_bytes)
{
    // Assemble complete memory settings
    oxnas_dma_device_settings_t mem_settings = oxnas_ram_dma_settings;
    mem_settings.address_ = (unsigned long)buffer;
    mem_settings.address_mode_ = OXNAS_DMA_MODE_INC;

    writel(encode_control_status(&oxnas_sata_dma_settings, &mem_settings),
        SATA_DMA_REGS_BASE + DMA_CTRL_STATUS); 
    writel(oxnas_sata_dma_settings.address_, SATA_DMA_REGS_BASE + DMA_BASE_SRC_ADR); 
    writel(mem_settings.address_, SATA_DMA_REGS_BASE + DMA_BASE_DST_ADR);
    writel(encode_final_eot(&oxnas_sata_dma_settings, &mem_settings, num_bytes),
        SATA_DMA_REGS_BASE + DMA_BYTE_CNT);

    dma_start();
}

static inline int dma_busy(void)
{
    return readl(SATA_DMA_REGS_BASE + DMA_CTRL_STATUS) & DMA_CTRL_STATUS_IN_PROGRESS;
}

static int wait_dma_not_busy(int device)
{
    unsigned int cleanup_required = 0;

	/* Poll for DMA completion */
	int loops = MAX_DMA_XFER_LOOPS;
	do {
		if (!dma_busy()) {
			break;
		}
		udelay(100);
	} while (--loops);

	if (!loops) {
		printf("wait_dma_not_busy() Timed out of wait for DMA not busy\n");
		cleanup_required = 1;
	}

    if (cleanup_required) {
        /* Abort DMA to make sure it has finished. */
        unsigned int ctrl_status = readl(SATA_DMA_CHANNEL + DMA_CTRL_STATUS);
        ctrl_status |= DMA_CTRL_STATUS_RESET;
        writel(ctrl_status, SATA_DMA_CHANNEL + DMA_CTRL_STATUS);

        // Wait for the channel to become idle - should be quick as should
        // finish after the next AHB single or burst transfer
        loops = MAX_DMA_ABORT_LOOPS;
        do {
            if (!dma_busy()) {
                break;
            }
            udelay(10);
        } while (--loops);

        if (!loops) {
            printf("wait_dma_not_busy() Timed out of wait for DMA channel abort\n");
        } else {
			/* Successfully cleanup the DMA channel */
			cleanup_required = 0;
		}

        // Deassert reset for the channel
        ctrl_status = readl(SATA_DMA_CHANNEL + DMA_CTRL_STATUS);
        ctrl_status &= ~DMA_CTRL_STATUS_RESET;
        writel(ctrl_status, SATA_DMA_CHANNEL + DMA_CTRL_STATUS);
    }

	return !cleanup_required;
}

/**
 * Possible that ATA status will not become not-busy, so must have timeout
 */
static unsigned int wait_not_busy(int device, unsigned long timeout_secs)
{
    int busy = 1;
    unsigned long loops = (timeout_secs * 1000) / 50;
    do {
        // Test the ATA status register BUSY flag
        if (!((*(sata_regs_base[device] + SATA_ORB2_OFF) >> SATA_COMMAND_BIT) & (1UL << ATA_STATUS_BSY_BIT))) {
            /* Not busy, so stop polling */
            busy = 0;
            break;
        }

        // Wait for 50mS before sampling ATA status register again
        udelay(50000);
    } while (--loops);

    return busy;
}

void oxnas_sata_output_data(int device, ulong *sect_buf, int words)
{
    /* Only permit accesses to disks found to be present during ide_preinit() */
    if (!disk_present[device]) {
        return;
    }

    /* Select the required internal SATA drive */
    device_select(device);

    /* Start the DMA channel sending data from the passed buffer to the SATA core */
    dma_start_write(sect_buf, words << 2);

	/* Don't know why we need this delay, but without it the wait for DMA not 
	   busy times soemtimes out, e.g. when saving environment to second disk */
	udelay(1000);

    /* Wait for DMA to finish */
    if (!wait_dma_not_busy(device)) {
		printf("Timed out of wait for DMA channel for SATA device %d to have in-progress clear\n", device);
	}

	/* Sata core should finish after DMA */
	if (wait_not_busy(device, 30)) {
		printf("Timed out of wait for SATA device %d to have BUSY clear\n", device);
	}
	if (!wait_no_error(device)) {
		printf("oxnas_sata_output_data() Wait for ATA no-error timed-out\n");
	}
}

void oxnas_sata_input_data(int device, ulong *sect_buf, int words)
{
    /* Only permit accesses to disks found to be present during ide_preinit() */
    if (!disk_present[device]) {
        return;
    }

    /* Select the required internal SATA drive */
    device_select(device);

    /* Start the DMA channel receiving data from the SATA core into the passed buffer */
    dma_start_read(sect_buf, words << 2);

	/* Sata core should finish before DMA */
	if (wait_not_busy(device, 30)) {
		printf("Timed out of wait for SATA device %d to have BUSY clear\n", device);
	}
	if (!wait_no_error(device)) {
		printf("oxnas_sata_output_data() Wait for ATA no-error timed-out\n");
	}

    /* Wait for DMA to finish */
    if (!wait_dma_not_busy(device)) {
		printf("Timed out of wait for DMA channel for SATA device %d to have in-progress clear\n", device);
	}
}

static u32 scr_read(int device, unsigned int sc_reg)
{
    /* Setup adr of required register. std regs start eight into async region */    
    *(sata_regs_base[device] + SATA_LINK_RD_ADDR) = sc_reg*4 + SATA_STD_ASYNC_REGS_OFF;

    /* Wait for data to be available */
    int loops = MAX_SRC_READ_LOOPS;
    do {
        if (*(sata_regs_base[device] + SATA_LINK_CONTROL) & 1UL) {
            break;
        }
        udelay(10);
    } while (--loops);

    if (!loops) {
        printf("scr_read() Timed out of wait for read completion\n");
    }

    /* Read the data from the async register */
    return *(sata_regs_base[device] + SATA_LINK_DATA);
}

static void scr_write(int device, unsigned int sc_reg, u32 val)
{
    /* Setup the data for the write */
    *(sata_regs_base[device] + SATA_LINK_DATA) = val;

    /* Setup adr of required register. std regs start eight into async region */    
    *(sata_regs_base[device] + SATA_LINK_WR_ADDR) = sc_reg*4 + SATA_STD_ASYNC_REGS_OFF;

    /* Wait for data to be written */
    int loops = MAX_SRC_WRITE_LOOPS;
    do {
        if (*(sata_regs_base[device] + SATA_LINK_CONTROL) & 1UL) {
            break;
        }
        udelay(10);
    } while (--loops);

    if (!loops) {
        printf("scr_write() Timed out of wait for write completion\n");
    }
}
extern void workaround5458(void);
extern void EnableSATAPhy(void);

#define PHY_LOOP_COUNT  25  /* Wait for upto 5 seconds for PHY to be found */
static int phy_reset(int device)
{
    int phy_status = 0;
    int loops = 0;

    workaround5458();
    scr_write(device, 0x60, 0x2988);
    scr_write(device, 0x70, 0x55629);

#ifdef FPGA
    /* The FPGA thinks it can do 3G when infact only 1.5G is possible, so limit
    it to Gen-1 SATA (1.5G) */ 
    scr_write(device, SATA_SCR_CONTROL, 0x311);  /* Issue phy wake & core reset */
    scr_read(device, SATA_SCR_STATUS);           /* Dummy read; flush */
    udelay(1000);
    scr_write(device, SATA_SCR_CONTROL, 0x310);  /* Issue phy wake & clear core reset */
#else
    udelay(500000);
    scr_write(device, SATA_SCR_CONTROL, 0x301);  /* Issue phy wake & core reset */
    scr_read(device, SATA_SCR_STATUS);           /* Dummy read; flush */
    udelay(1000);
    scr_write(device, SATA_SCR_CONTROL, 0x300);  /* Issue phy wake & clear core reset */
#endif

    /* Wait for upto 5 seconds for PHY to become ready */
    do {
        udelay(200000);
        if ((scr_read(device, SATA_SCR_STATUS) & 0xf) ==3) {
            scr_write(device, SATA_SCR_ERROR, ~0);
            phy_status = 1;
            break;
        }
        //printf("No SATA PHY found status:0x%x\n", scr_read(device, SATA_SCR_STATUS));
    } while (++loops < PHY_LOOP_COUNT);

    if (phy_status) {
        udelay(500000); /* wait half a second */
    }


    return phy_status;
}

#define FIS_LOOP_COUNT  25  /* Wait for upto 5 seconds for FIS to be received */
static int wait_FIS(int device)
{
    int status = 0;
    int loops = 0;

    do {
        udelay(200000);
        if 	(oxnas_sata_inb(device, ATA_PORT_NSECT) > 0) {
            status = 1;
            break;
        }
    } while (++loops < FIS_LOOP_COUNT);

    return status;
}

int ide_preinit(void)
{
    int num_disks_found = 0;

    /* Initialise records of which disks are present to all present */
    int i;
    for (i=0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++) {
        disk_present[i] = 1;
    }
    
    /* Block reset SATA and DMA cores */
    *(volatile u32*)SYS_CTRL_RSTEN_SET_CTRL = (1UL << SYS_CTRL_RSTEN_SATA_BIT) |
                                              (1UL << SYS_CTRL_RSTEN_SATA_LINK_BIT) |
                                              (1UL << SYS_CTRL_RSTEN_SATA_PHY_BIT) |
                                              (1UL << SYS_CTRL_RSTEN_SGDMA_BIT);

    /* Enable clocks to SATA and DMA cores */
    *(volatile u32*)SYS_CTRL_CKEN_SET_CTRL = (1UL << SYS_CTRL_CKEN_SATA_BIT);
    *(volatile u32*)SYS_CTRL_CKEN_SET_CTRL = (1UL << SYS_CTRL_CKEN_DMA_BIT);
    
    udelay(5000);
    *(volatile u32*)SYS_CTRL_RSTEN_CLR_CTRL = (1UL << SYS_CTRL_RSTEN_SATA_PHY_BIT);
    udelay(50);
    *(volatile u32*)SYS_CTRL_RSTEN_CLR_CTRL = (1UL << SYS_CTRL_RSTEN_SATA_LINK_BIT) |
                                              (1UL << SYS_CTRL_RSTEN_SATA_BIT);
    udelay(50);
    *(volatile u32*)SYS_CTRL_RSTEN_CLR_CTRL = (1UL << SYS_CTRL_RSTEN_SGDMA_BIT);
    udelay(100);
    /* Apply the Synopsis SATA PHY workarounds */
    EnableSATAPhy();
    udelay(10000);

    /* disable and clear core interrupts */
    *((unsigned long*)SATA_HOST_REGS_BASE + SATA_INT_ENABLE_CLR_OFF) = ~0UL;
    *((unsigned long*)SATA_HOST_REGS_BASE + SATA_INT_CLR_OFF) = ~0UL;

    int device;
    for (device = 0; device < CONFIG_SYS_SATA_MAX_DEVICE; device++) {
        int found = 0;
        int retries = 1;

        /* Disable SATA interrupts */
        *(sata_regs_base[device] + SATA_INT_ENABLE_CLR_OFF) = ~0UL;

        /* Clear any pending SATA interrupts */
        *(sata_regs_base[device] + SATA_INT_CLR_OFF) = ~0UL;

        do {
            /* clear sector count register for FIS detection */
            oxnas_sata_outb(device, ATA_PORT_NSECT, 0);
    
            /* Get the PHY working */
            if (!phy_reset(device)) {
                printf("SATA PHY not ready for device %d\n", device);
                break;
            }

            if (!wait_FIS(device)) {
                printf("No FIS received from device %d\n", device);
            } else {
                if ((scr_read(device, SATA_SCR_STATUS) & 0xf) == 0x3) {
                    if (wait_not_busy(device, 30)) {
                        printf("Timed out of wait for SATA device %d to have BUSY clear\n", device);
                    } else {
                        ++num_disks_found;
                        found = 1;
                    }
                } else {
                    printf("No SATA device %d found, PHY status = 0x%08x\n",
                            device, scr_read(device, SATA_SCR_STATUS));
                }
                break;
            }
        } while (retries--) ;

        /* Record whether disk is present, so won't attempt to access it later */
        disk_present[device] = found;
    }

    /* post disk detection clean-up */
    for (device = 0; device < CONFIG_SYS_SATA_MAX_DEVICE; device++) {
        if ( disk_present[device] ) {
            /* set as ata-5 (28-bit) */
            *(sata_regs_base[device] + SATA_DRIVE_CONTROL_OFF) = 0UL;
            
            /* clear phy/link errors */
            scr_write(device, SATA_SCR_ERROR, ~0);
            
            /* clear host errors */
            *(sata_regs_base[device] + SATA_CONTROL_OFF) |= SATA_SCTL_CLR_ERR;
            
            /* clear interrupt register as this clears the error bit in the IDE 
            status register */
            *(sata_regs_base[device] + SATA_INT_CLR_OFF) = ~0UL;
        }
    }    
    

    return !num_disks_found;
}

static void __inline__ ide_outb(int dev, int port, unsigned char val)
{
    oxnas_sata_outb(dev, port, val);
}

static unsigned char __inline__ ide_inb(int dev, int port)
{
    return oxnas_sata_inb(dev, port);
}

#define input_swap_data(x,y,z) input_data(x,y,z)

static void input_data(int dev, ulong *sect_buf, int words)
{
    oxnas_sata_input_data(dev, sect_buf, words);
}

static void output_data(int dev, ulong *sect_buf, int words)
{
    oxnas_sata_output_data(dev, sect_buf, words);
}

static void byte_swap_and_trim(char* buf)
{
    char *src = buf;

    // Swap bytes in 16-bit words
    while ((*src != '\0') && (*(src+1) != '\0')) {
        char tmp = *(src+1);
        *(src+1) = *src;
        *src = tmp;
        src += 2;
    }

    // Trim leading spaces
    src = buf;
    while (*src == ' ') {
        ++src;
    }
    if (src != buf) {
        memcpy(buf, src, strlen(src));
        buf[strlen(buf) - (src-buf)] = '\0';
    }
}

/*
 * copy src to dest, skipping leading and trailing blanks and null
 * terminate the string
 * "len" is the size of available memory including the terminating '\0'
 */
static void ident_cpy (unsigned char *dst, unsigned char *src, unsigned int len)
{
	unsigned char *end, *last;

	last = dst;
	end  = src + len - 1;

	/* reserve space for '\0' */
	if (len < 2)
		goto OUT;

	/* skip leading white space */
	while ((*src) && (src<end) && (*src==' '))
		++src;

	/* copy string, omitting trailing white space */
	while ((*src) && (src<end)) {
		*dst++ = *src;
		if (*src++ != ' ')
			last = dst;
	}
OUT:
	*last = '\0';
}

/* ------------------------------------------------------------------------- */

/*
 * Wait until Busy bit is off, or timeout (in ms)
 * Return last status
 */
static uchar ide_wait (int dev, ulong t)
{
	ulong delay = 10 * t;		/* poll every 100 us */
	uchar c;

	while ((c = ide_inb(dev, ATA_STATUS)) & ATA_STAT_BUSY) {
		udelay (100);
		if (delay-- == 0) {
			break;
		}
	}
	return (c);
}


static void ide_ident (block_dev_desc_t *dev_desc)
{
	ulong iobuf[ATA_SECTORWORDS];
	unsigned char c;
    unsigned int i;
	hd_driveid_t *iop = (hd_driveid_t *)iobuf;

#ifdef CONFIG_ATAPI
	int retries = 0;
	int do_retry = 0;
#endif

	int device;
	device=dev_desc->dev;
	printf ("  Device %d: ", device);

    for ( i=0; i < ATA_SECTORWORDS; ++i) {
        iobuf[i] = 0;
    }
    
	ide_led (DEVICE_LED(device), 1);	/* LED on	*/
	/* Select device
	 */
	ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	dev_desc->if_type=IF_TYPE_IDE;
#ifdef CONFIG_ATAPI

    do_retry = 0;
    retries = 0;

    /* Warning: This will be tricky to read */
    while (retries <= 1) {
	/* check signature */
	if ((ide_inb(device,ATA_SECT_CNT) == 0x01) &&
		 (ide_inb(device,ATA_SECT_NUM) == 0x01) &&
		 (ide_inb(device,ATA_CYL_LOW)  == 0x14) &&
		 (ide_inb(device,ATA_CYL_HIGH) == 0xEB)) {
		/* ATAPI Signature found */
		dev_desc->if_type=IF_TYPE_ATAPI;
		/* Start Ident Command
		 */
		ide_outb (device, ATA_COMMAND, ATAPI_CMD_IDENT);
		/*
		 * Wait for completion - ATAPI devices need more time
		 * to become ready
		 */
		c = ide_wait (device, ATAPI_TIME_OUT);
	} else
#endif
	{
		/* Start Ident Command
		 */
		ide_outb (device, ATA_COMMAND, ATA_CMD_IDENT);

		/* Wait for completion
		 */
		c = ide_wait (device, IDE_TIME_OUT);
	}
	ide_led (DEVICE_LED(device), 0);	/* LED off	*/

	if (((c & ATA_STAT_DRQ) == 0) ||
	    ((c & (ATA_STAT_FAULT|ATA_STAT_ERR)) != 0) ) {
#ifdef CONFIG_ATAPI
			{
				/* Need to soft reset the device in case it's an ATAPI...  */
				printf("Retrying...\n");
				ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
				udelay(100000);
				ide_outb (device, ATA_COMMAND, 0x08);
				udelay (500000);	/* 500 ms */
			}
		/* Select device
		 */
		ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
		retries++;
#else
		return;
#endif
	}
#ifdef CONFIG_ATAPI
	else
		break;
    }	/* see above - ugly to read */

	if (retries == 2) /* Not found */
		return;
#endif

	input_swap_data (device, iobuf, ATA_SECTORWORDS);

	ident_cpy((unsigned char*)dev_desc->revision, iop->fw_rev, sizeof(dev_desc->revision));
	ident_cpy((unsigned char*)dev_desc->vendor, iop->model, sizeof(dev_desc->vendor));
	ident_cpy((unsigned char*)dev_desc->product, iop->serial_no, sizeof(dev_desc->product));

#ifdef __LITTLE_ENDIAN
	/*
	 * firmware revision, model number and product have Big Endian Byte
	 * order in Word. Convert both to little endian.
	 *
	 * See CF+ and CompactFlash Specification Revision 2.0:
	 * 6.2.1.6: Identfy Drive, Table 39 for more details
	 */

	byte_swap_and_trim(dev_desc->revision);
	byte_swap_and_trim(dev_desc->vendor);
	byte_swap_and_trim(dev_desc->product);
#endif /* __LITTLE_ENDIAN */

	if ((iop->config & 0x0080)==0x0080)
		dev_desc->removable = 1;
	else
		dev_desc->removable = 0;

#ifdef CONFIG_ATAPI
	if (dev_desc->if_type==IF_TYPE_ATAPI) {
		atapi_inquiry(dev_desc);
		return;
	}
#endif /* CONFIG_ATAPI */

#ifdef __BIG_ENDIAN
	/* swap shorts */
	dev_desc->lba = (iop->lba_capacity << 16) | (iop->lba_capacity >> 16);
#else	/* ! __BIG_ENDIAN */
	/*
	 * do not swap shorts on little endian
	 *
	 * See CF+ and CompactFlash Specification Revision 2.0:
	 * 6.2.1.6: Identfy Drive, Table 39, Word Address 57-58 for details.
	 */
	dev_desc->lba = iop->lba_capacity;
#endif	/* __BIG_ENDIAN */

#ifdef CONFIG_LBA48
	if (iop->command_set_2 & 0x0400) { /* LBA 48 support */
		dev_desc->lba48 = 1;
		dev_desc->lba = (unsigned long long)iop->lba48_capacity[0] |
						  ((unsigned long long)iop->lba48_capacity[1] << 16) |
						  ((unsigned long long)iop->lba48_capacity[2] << 32) |
						  ((unsigned long long)iop->lba48_capacity[3] << 48);
	} else {
		dev_desc->lba48 = 0;
	}
#endif /* CONFIG_LBA48 */
	/* assuming HD */
	dev_desc->type=DEV_TYPE_HARDDISK;
	dev_desc->blksz=ATA_BLOCKSIZE;
	dev_desc->lun=0; /* just to fill something in... */
}

ulong sata_read (int device, lbaint_t blknr, ulong blkcnt, ulong *buffer)
{
	ulong n = 0;
	unsigned char c;
	unsigned char pwrsave=0; /* power save */
#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr & (((uint64_t)0x0000fffff) << 28) ) {
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif
	printf ("ide_read dev %d start %qX, blocks %lX buffer at %lX\n",
		device, blknr, blkcnt, (ulong)buffer);

	ide_led (DEVICE_LED(device), 1);	/* LED on	*/

	/* Select device
	 */
	ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = ide_wait (device, IDE_TIME_OUT);

	if (c & ATA_STAT_BUSY) {
		printf ("IDE read: device %d not ready\n", device);
		goto IDE_READ_E;
	}

	/* first check if the drive is in Powersaving mode, if yes,
	 * increase the timeout value */
	ide_outb (device, ATA_COMMAND,  ATA_CMD_CHK_PWR);
	udelay (50);

	c = ide_wait (device, IDE_TIME_OUT);	/* can't take over 500 ms */

	if (c & ATA_STAT_BUSY) {
		printf ("IDE read: device %d not ready\n", device);
		goto IDE_READ_E;
	}
	if ((c & ATA_STAT_ERR) == ATA_STAT_ERR) {
		printf ("No Powersaving mode %X\n", c);
	} else {
		c = ide_inb(device,ATA_SECT_CNT);
		printf("Powersaving %02X\n",c);
		if(c==0)
			pwrsave=1;
	}


	while (blkcnt-- > 0) {

		c = ide_wait (device, IDE_TIME_OUT);

		if (c & ATA_STAT_BUSY) {
			printf ("IDE read: device %d not ready\n", device);
			break;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			ide_outb (device, ATA_SECT_CNT, 0);
			ide_outb (device, ATA_LBA_LOW,	(blknr >> 24) & 0xFF);
			ide_outb (device, ATA_LBA_MID,	(blknr >> 32) & 0xFF);
			ide_outb (device, ATA_LBA_HIGH, (blknr >> 40) & 0xFF);
		}
#endif
		ide_outb (device, ATA_SECT_CNT, 1);
		ide_outb (device, ATA_LBA_LOW,  (blknr >>  0) & 0xFF);
		ide_outb (device, ATA_LBA_MID,  (blknr >>  8) & 0xFF);
		ide_outb (device, ATA_LBA_HIGH, (blknr >> 16) & 0xFF);

#ifdef CONFIG_LBA48
		if (lba48) {
			ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device) );
			ide_outb (device, ATA_COMMAND, ATA_CMD_READ_EXT);

		} else
#endif
		{
			ide_outb (device, ATA_DEV_HD,   ATA_LBA		|
						    ATA_DEVICE(device)	|
						    ((blknr >> 24) & 0xF) );
			ide_outb (device, ATA_COMMAND,  ATA_CMD_READ);
		}

		udelay (50);

		if(pwrsave) {
			c = ide_wait (device, IDE_SPIN_UP_TIME_OUT);	/* may take up to 4 sec */
			pwrsave=0;
		} else {
			c = ide_wait (device, IDE_TIME_OUT);	/* can't take over 500 ms */
		}

		if ((c&(ATA_STAT_DRQ|ATA_STAT_BUSY|ATA_STAT_ERR)) != ATA_STAT_DRQ) {
#if defined(CONFIG_SYS_64BIT_LBA)
			printf ("Error (no IRQ) dev %d blk %qd: status 0x%02x\n",
				device, blknr, c);
#else
			printf ("Error (no IRQ) dev %d blk %ld: status 0x%02x\n",
				device, (ulong)blknr, c);
#endif
			break;
		}

		input_data (device, buffer, ATA_SECTORWORDS);
		(void) ide_inb (device, ATA_STATUS);	/* clear IRQ */

		++n;
		++blknr;
		buffer += ATA_SECTORWORDS;
	}
IDE_READ_E:
	ide_led (DEVICE_LED(device), 0);	/* LED off	*/
	return (n);
}

/* ------------------------------------------------------------------------- */


ulong sata_write (int device, lbaint_t blknr, ulong blkcnt, ulong *buffer)
{
	ulong n = 0;
	unsigned char c;
#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr & (((uint64_t)0x0000fffff) << 28)) {
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif

	ide_led (DEVICE_LED(device), 1);	/* LED on	*/

	/* Select device
	 */
	ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	while (blkcnt-- > 0) {

		c = ide_wait (device, IDE_TIME_OUT);

		if (c & ATA_STAT_BUSY) {
			printf ("IDE read: device %d not ready\n", device);
			goto WR_OUT;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			ide_outb (device, ATA_SECT_CNT, 0);
			ide_outb (device, ATA_LBA_LOW,	(blknr >> 24) & 0xFF);
			ide_outb (device, ATA_LBA_MID,	(blknr >> 32) & 0xFF);
			ide_outb (device, ATA_LBA_HIGH, (blknr >> 40) & 0xFF);
		}
#endif
		ide_outb (device, ATA_SECT_CNT, 1);
		ide_outb (device, ATA_LBA_LOW,  (blknr >>  0) & 0xFF);
		ide_outb (device, ATA_LBA_MID,  (blknr >>  8) & 0xFF);
		ide_outb (device, ATA_LBA_HIGH, (blknr >> 16) & 0xFF);

#ifdef CONFIG_LBA48
		if (lba48) {
			ide_outb (device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device) );
			ide_outb (device, ATA_COMMAND,	ATA_CMD_WRITE_EXT);

		} else
#endif
		{
			ide_outb (device, ATA_DEV_HD,   ATA_LBA		|
						    ATA_DEVICE(device)	|
						    ((blknr >> 24) & 0xF) );
			ide_outb (device, ATA_COMMAND,  ATA_CMD_WRITE);
		}

		udelay (50);

		c = ide_wait (device, IDE_TIME_OUT);	/* can't take over 500 ms */

		if ((c&(ATA_STAT_DRQ|ATA_STAT_BUSY|ATA_STAT_ERR)) != ATA_STAT_DRQ) {
#if defined(CONFIG_SYS_64BIT_LBA)
			printf ("Error (no IRQ) dev %d blk %qd: status 0x%02x\n",
				device, blknr, c);
#else
			printf ("Error (no IRQ) dev %d blk %ld: status 0x%02x\n",
				device, (ulong)blknr, c);
#endif
			goto WR_OUT;
		}

		output_data (device, buffer, ATA_SECTORWORDS);
		c = ide_inb (device, ATA_STATUS);	/* clear IRQ */
		++n;
		++blknr;
		buffer += ATA_SECTORWORDS;
	}
WR_OUT:
	ide_led (DEVICE_LED(device), 0);	/* LED off	*/
	return (n);
}

int sata_identify(int dev)
{
#ifdef CONFIG_SATA
    sata_dev_desc[dev].type=DEV_TYPE_UNKNOWN;
    sata_dev_desc[dev].if_type=IF_TYPE_IDE;
    sata_dev_desc[dev].dev=dev;
    sata_dev_desc[dev].part_type=PART_TYPE_UNKNOWN;
    sata_dev_desc[dev].blksz=0;
    sata_dev_desc[dev].lba=0;
    ide_ident(&sata_dev_desc[dev]);
#ifdef CONFIG_PARTITIONS
    if ((sata_dev_desc[dev].lba > 0) && (sata_dev_desc[dev].blksz > 0)) {
        init_part(&sata_dev_desc[dev]);    /* initialize partition type */
    }
#endif
#endif
    return 0;
}

int scan_sata (int dev);

int init_sata(int dev)
{
    static int res = 1;
    static u8 init_done = 0;
    if(init_done)
    {
        return res;
    }
    
    init_done = 1;
    
    if(ide_preinit())
    {
        return res = 1;
    }
    
    if(scan_sata(0))
    {
        return res = 1;
    }
    
    return res = 0;
}

/* Check if device is connected to port */
int sata_bus_probe (int portno)
{
    if(portno != 0)
    {
        return 0;
    }
    else
    {
        return 1;
	}
}


int scan_sata (int dev)
{
	/* A bit brain dead, but the code has a legacy */
    if(dev != 0)
    {
        return 0;
    }
    
    sata_identify(0);
    
	return 1;
}

#endif
