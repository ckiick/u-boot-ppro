#include <common.h>
#include <asm/arch-oxnas820/regs.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE; /* convert to MBytes */

    gd->bd->bi_dram[0].start = PHYS_SDRAM_1_PA;
    
    gd->ram_size = PHYS_SDRAM_1_SIZE;

    return 0;
}
