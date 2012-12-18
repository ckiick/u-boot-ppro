/*
 * Copyright (C) 2000-2005, DENX Software Engineering
 *		Wolfgang Denk <wd@denx.de>
 * Copyright (C) Procsys. All rights reserved.
 *		Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
 *		Dave Liu <daveliu@freescale.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <part.h>
#include <sata.h>

static int sata_curr_device = -1;
block_dev_desc_t sata_dev_desc[CONFIG_SYS_SATA_MAX_DEVICE];

int __sata_initialize(void)
{
	int rc;
	int i;

	for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; i++) {
		memset(&sata_dev_desc[i], 0, sizeof(struct block_dev_desc));
		sata_dev_desc[i].if_type = IF_TYPE_SATA;
		sata_dev_desc[i].dev = i;
		sata_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
		sata_dev_desc[i].type = DEV_TYPE_HARDDISK;
		sata_dev_desc[i].lba = 0;
		sata_dev_desc[i].blksz = 512;
		sata_dev_desc[i].block_read = sata_read;
		sata_dev_desc[i].block_write = sata_write;

		rc = init_sata(i);
		if (!rc) {
			rc = scan_sata(i);
			if (!rc && (sata_dev_desc[i].lba > 0) &&
				(sata_dev_desc[i].blksz > 0))
				init_part(&sata_dev_desc[i]);
		}
	}
	sata_curr_device = 0;
	return rc;
}
int sata_initialize(void) __attribute__((weak,alias("__sata_initialize")));

#ifdef CONFIG_PARTITIONS
block_dev_desc_t *sata_get_dev(int dev)
{
	return (dev < CONFIG_SYS_SATA_MAX_DEVICE) ? &sata_dev_desc[dev] : NULL;
}
#endif

static int do_sata(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = 0;

	if (argc == 2 && strcmp(argv[1], "init") == 0)
		return sata_initialize();

	/* If the user has not yet run `sata init`, do it now */
	if (sata_curr_device == -1)
		if (sata_initialize())
			return 1;

	switch (argc) {
	case 0:
	case 1:
		return CMD_RET_USAGE;
	case 2:
		if (strncmp(argv[1],"inf", 3) == 0) {
			int i;
			putc('\n');
			for (i = 0; i < CONFIG_SYS_SATA_MAX_DEVICE; ++i) {
				if (sata_dev_desc[i].type == DEV_TYPE_UNKNOWN)
					continue;
				printf ("SATA device %d: ", i);
				dev_print(&sata_dev_desc[i]);
			}
			return 0;
		} else if (strncmp(argv[1],"dev", 3) == 0) {
			if ((sata_curr_device < 0) || (sata_curr_device >= CONFIG_SYS_SATA_MAX_DEVICE)) {
				puts("\nno SATA devices available\n");
				return 1;
			}
			printf("\nSATA device %d: ", sata_curr_device);
			dev_print(&sata_dev_desc[sata_curr_device]);
			return 0;
		} else if (strncmp(argv[1],"part",4) == 0) {
			int dev, ok;

			for (ok = 0, dev = 0; dev < CONFIG_SYS_SATA_MAX_DEVICE; ++dev) {
				if (sata_dev_desc[dev].part_type != PART_TYPE_UNKNOWN) {
					++ok;
					if (dev)
						putc ('\n');
					print_part(&sata_dev_desc[dev]);
				}
			}
			if (!ok) {
				puts("\nno SATA devices available\n");
				rc ++;
			}
			return rc;
		}
		return CMD_RET_USAGE;
	case 3:
		if (strncmp(argv[1], "dev", 3) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			printf("\nSATA device %d: ", dev);
			if (dev >= CONFIG_SYS_SATA_MAX_DEVICE) {
				puts ("unknown device\n");
				return 1;
			}
			dev_print(&sata_dev_desc[dev]);

			if (sata_dev_desc[dev].type == DEV_TYPE_UNKNOWN)
				return 1;

			sata_curr_device = dev;

			puts("... is now current device\n");

			return 0;
		} else if (strncmp(argv[1], "part", 4) == 0) {
			int dev = (int)simple_strtoul(argv[2], NULL, 10);

			if (sata_dev_desc[dev].part_type != PART_TYPE_UNKNOWN) {
				print_part(&sata_dev_desc[dev]);
			} else {
				printf("\nSATA device %d not available\n", dev);
				rc = 1;
			}
			return rc;
		}
		return CMD_RET_USAGE;

	default: /* at least 4 args */
		if (strcmp(argv[1], "read") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;
			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA read: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = sata_read(sata_curr_device, blk, cnt, (u32 *)addr);

			/* flush cache after read */
			flush_cache(addr, cnt * sata_dev_desc[sata_curr_device].blksz);

			printf("%ld blocks read: %s\n",
				n, (n==cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else if (strcmp(argv[1], "write") == 0) {
			ulong addr = simple_strtoul(argv[2], NULL, 16);
			ulong cnt = simple_strtoul(argv[4], NULL, 16);
			ulong n;

			lbaint_t blk = simple_strtoul(argv[3], NULL, 16);

			printf("\nSATA write: device %d block # %ld, count %ld ... ",
				sata_curr_device, blk, cnt);

			n = sata_write(sata_curr_device, blk, cnt, (u32 *)addr);

			printf("%ld blocks written: %s\n",
				n, (n == cnt) ? "OK" : "ERROR");
			return (n == cnt) ? 0 : 1;
		} else {
			return CMD_RET_USAGE;
		}

		return rc;
	}
}

/* I hate duplicating code, but that seems to be the convention here.*/
int do_sataboot(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev, part = 0;
	ulong addr, cnt;
	disk_partition_t info;
	image_header_t *hdr;

#if defined(CONFIG_FIT)
	const void *fit_hdr = NULL;
#endif

	bootstage_mark(BOOTSTAGE_ID_IDE_START);
	switch (argc) {
	case 1:
		addr = CONFIG_SYS_LOAD_ADDR;
		boot_device = getenv("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	default:
		bootstage_error(BOOTSTAGE_ID_IDE_ADDR);
		return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_ADDR);

	if (!boot_device) {
		puts("\n** No boot device **\n");
		bootstage_error(BOOTSTAGE_ID_IDE_BOOT_DEVICE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_BOOT_DEVICE);

	dev = simple_strtoul(boot_device, &ep, 16);

	if (dev < 0 || dev >= CONFIG_SYS_SATA_MAX_DEVICE) {
		printf("\n** Device %d invalid\n", dev);
		bootstage_error(BOOTSTAGE_ID_IDE_BOOT_DEVICE);
		return 1;
	}

	if (sata_dev_desc[dev].type == DEV_TYPE_UNKNOWN) {
		printf("\n** Device %d not available\n", dev);
		bootstage_error(BOOTSTAGE_ID_IDE_TYPE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_TYPE);

	if (*ep) {
		if (*ep != ':') {
			puts("\n** Invalid boot device, use `dev[:part]' **\n");
			bootstage_error(BOOTSTAGE_ID_IDE_PART);
			return 1;
		}
		part = simple_strtoul(++ep, NULL, 16);
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART);

	if (get_partition_info(&sata_dev_desc[dev], part, &info)) {
		bootstage_error(BOOTSTAGE_ID_IDE_PART_INFO);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_INFO);

	if ((strncmp((char *)info.type, BOOT_PART_TYPE, sizeof(info.type)) != 0)
	    &&
	    (strncmp((char *)info.type, BOOT_PART_COMP, sizeof(info.type)) != 0)
	   ) {
		printf("\n** Invalid partition type \"%.32s\"" " (expect \""
			BOOT_PART_TYPE "\")\n",
			info.type);
		bootstage_error(BOOTSTAGE_ID_IDE_PART_TYPE);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_TYPE);

	printf("\nLoading from IDE device %d, partition %d: "
	       "Name: %.32s  Type: %.32s\n", dev, part, info.name, info.type);

	debug("First Block: %ld,  # of blocks: %ld, Block Size: %ld\n",
	      info.start, info.size, info.blksz);

	if (sata_dev_desc[dev].
	    block_read(dev, info.start, 1, (ulong *) addr) != 1) {
		printf("** Read error on %d:%d\n", dev, part);
		bootstage_error(BOOTSTAGE_ID_IDE_PART_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_PART_READ);

	switch (genimg_get_format((void *) addr)) {
	case IMAGE_FORMAT_LEGACY:
		hdr = (image_header_t *) addr;

		bootstage_mark(BOOTSTAGE_ID_IDE_FORMAT);

		if (!image_check_hcrc(hdr)) {
			puts("\n** Bad Header Checksum **\n");
			bootstage_error(BOOTSTAGE_ID_IDE_CHECKSUM);
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_IDE_CHECKSUM);

		image_print_contents(hdr);

		cnt = image_get_image_size(hdr);
		break;
#if defined(CONFIG_FIT)
	case IMAGE_FORMAT_FIT:
		fit_hdr = (const void *) addr;
		puts("Fit image detected...\n");

		cnt = fit_get_size(fit_hdr);
		break;
#endif
	default:
		bootstage_error(BOOTSTAGE_ID_IDE_FORMAT);
		puts("** Unknown image type\n");
		return 1;
	}

	cnt += info.blksz - 1;
	cnt /= info.blksz;
	cnt -= 1;

	if (sata_dev_desc[dev].block_read(dev, info.start + 1, cnt,
					 (ulong *)(addr + info.blksz)) != cnt) {
		printf("** Read error on %d:%d\n", dev, part);
		bootstage_error(BOOTSTAGE_ID_IDE_READ);
		return 1;
	}
	bootstage_mark(BOOTSTAGE_ID_IDE_READ);

#if defined(CONFIG_FIT)
	/* This cannot be done earlier, we need complete FIT image in RAM first */
	if (genimg_get_format((void *) addr) == IMAGE_FORMAT_FIT) {
		if (!fit_check_format(fit_hdr)) {
			bootstage_error(BOOTSTAGE_ID_IDE_FIT_READ);
			puts("** Bad FIT image format\n");
			return 1;
		}
		bootstage_mark(BOOTSTAGE_ID_IDE_FIT_READ_OK);
		fit_print_contents(fit_hdr);
	}
#endif

	/* Loading ok, update default load address */

	load_addr = addr;

	return bootm_maybe_autostart(cmdtp, argv[0]);
}

U_BOOT_CMD(
	sata, 5, 1, do_sata,
	"SATA sub system",
	"sata init - init SATA sub system\n"
	"sata info - show available SATA devices\n"
	"sata device [dev] - show or set current device\n"
	"sata part [dev] - print partition table\n"
	"sata read addr blk# cnt\n"
	"sata write addr blk# cnt"
);

U_BOOT_CMD(sataboot, 3, 1, do_sataboot,
	   "boot from SATA device", "loadAddr dev[:part]");

