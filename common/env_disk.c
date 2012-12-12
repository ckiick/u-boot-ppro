/*
 * (C) Copyright 2006
 * Oxford Semiconductor Ltd
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Modified by Chris Kiick for u-boot 2012.07 7 dec 2012
 * CONFIG_ENV_OFFSET_REDUND is supported
 * CONFIG_SYS_REDUNDAND_ENVIRONMENT is NOT supported.
 * ENV_IS_EMBEDDED is not supported.
 * CONFIG_ENV_OFFSET[_REDUND] is in bytes from start of device, NOT sectors.
 *   Must be on a sector boundary, though.
 * NB: original code copied env from disk to SRAM and then relocated it
 * to RAM.  This was done because disks were available to read at init
 * time but relocation was not complete.  New startup sequence doesn't
 * do that, so we have to read at relocation time.  Might as well just
 * copy to the final destination.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <malloc.h>
#include <errno.h>
// something else for block dev interface. #include <ide.h>

#if defined(CONFIG_SYS_REDUNDAND_ENVIRONMENT) /* sic */
#error Disk environment cannot handle env_t with flags
#endif

#if defined(ENV_IS_EMBEDDED)
#error Disk environment does not support embedded environment
#endif

#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
        (CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

env_t *env_ptr = NULL;
char *env_name_spec = "Disk";

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec(int index)
{
    return *((uchar *)(gd->env_addr + index));
}

/*
 * called before SATA subsystem is initialized.  Just use default
 * until we are able to read from disk in the relocate spec func.
 */
int env_init(void)
{
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;	/* success! */
}

/*
 * now we can use a disk, and get the real environment.
 */
void env_relocate_spec(void)
{
#if defined(CONFIG_CMD_SATA)
extern void sata_initialize(void);
#endif
    block_dev_desc_t *dev_desc = NULL;
    int blksread;
    env_t env;

    char envbuf[CONFIG_ENV_SIZE];
    env_t *envptr = (env_t *)envbuf;

    /* try to find the device. */
    dev_desc = get_dev(CONFIG_SYS_DISK_ENV_INTERFACE, CONFIG_SYS_DISK_ENV_DEV);
    if (dev_desc == NULL) {
	printf("Cannot find device %s%d\n", CONFIG_SYS_DISK_ENV_INTERFACE, CONFIG_SYS_DISK_ENV_DEV);
	set_default_env(NULL);
	return;
    }

/* at this point we have to init the device. no generic way to do it. */
#if defined(CONFIG_CMD_SATA)
    sata_initialize();
#else
#error Disk env currently only implemented for sata devices
#endif

/* before sanity checking, see if the device is ready */
    if ((dev_desc->blksz == 0) || (dev_desc->lba == 0) || (dev_desc->type == DEV_TYPE_UNKNOWN)) {
	printf("device %s %d is not available\n", CONFIG_SYS_DISK_ENV_INTERFACE, CONFIG_SYS_DISK_ENV_DEV);
	set_default_env(NULL);
	return;
    }
    /* some sanity checking, just in case. */
    if (CONFIG_ENV_SIZE % dev_desc->blksz) {
	printf("Env size (%dK) is not a multiple of block size (%luK)\n", CONFIG_ENV_SIZE >> 10, dev_desc->blksz >> 10);
	set_default_env(NULL);
	return;
    }
    if (CONFIG_ENV_OFFSET % dev_desc->blksz) {
	printf("Env offset (%08x) is not on a block boundary\n", CONFIG_ENV_OFFSET);
	set_default_env(NULL);
	return;
    }

    /* now try to read the env block */
    blksread = dev_desc->block_read(dev_desc->dev, CONFIG_ENV_OFFSET / dev_desc->blksz, CONFIG_ENV_SIZE / dev_desc->blksz, (char *)&env);
    if (blksread == CONFIG_ENV_SIZE / 512) {
	if (crc32(0, env.data, ENV_SIZE) == env.crc) {
	    gd->env_valid = 1;
	    env_import((char *)&env, 0);
	    return;
	} else {
	    printf("Bad CRC on environment area\n");
	}
    } else {
	printf("Failed to read environment area\n");
    }

#ifdef CONFIG_ENV_OFFSET_REDUND
    printf("Failed to read primary environment, trying redundant copy.\n"); 
    blksread = dev_desc->block_read(dev_desc->dev, CONFIG_ENV_OFFSET_REDUND / dev_desc->blksz, CONFIG_ENV_SIZE / dev_desc->blksz, envbuf);
    if (blksread == CONFIG_ENV_SIZE / 512) {
	if (crc32(0, envptr->data, ENV_SIZE) == envptr->crc) {
	    gd->env_valid = 1;
	    env_import(envbuf, 0);
	    return;
	} else {
	    printf("Bad crc on redundant environment area\n");
	}
    } else {
	printf("Failed to read redundant environment area\n");
    }
#endif

    set_default_env("!No good environment found\n");
}

#ifdef CONFIG_CMD_SAVEENV
int saveenv(void)
{
    env_t env;
    size_t envlen;
    unsigned char *envbuf = env.data;
    block_dev_desc_t *dev_desc = NULL;
    int blkswrote;
    int rv = 0;

    envlen = hexport_r(&env_htab, '\0', &envbuf, ENV_SIZE, 0, NULL);
    if (envlen < 0) {
	printf("Failed to export environment: errno = %d\n", errno);
	return 1;
    }
    env.crc = crc32(0, envbuf, ENV_SIZE);

    dev_desc = get_dev(CONFIG_SYS_DISK_ENV_INTERFACE, CONFIG_SYS_DISK_ENV_DEV);
    if (dev_desc == NULL) {
	printf("Cannot find device %s%d\n", CONFIG_SYS_DISK_ENV_INTERFACE, CONFIG_SYS_DISK_ENV_DEV);
	return 1;
    }

    blkswrote = dev_desc->block_write(dev_desc->dev, CONFIG_ENV_OFFSET / dev_desc->blksz, CONFIG_ENV_SIZE / dev_desc->blksz, (char *)&env);
    if (blkswrote != CONFIG_ENV_SIZE / 512) {
	printf("Failed to write environment to disk at offset %d\n", CONFIG_ENV_OFFSET);
	rv = 1;
    }

#ifdef CONFIG_ENV_OFFSET_REDUND
    blkswrote = dev_desc->block_write(dev_desc->dev, CONFIG_ENV_OFFSET_REDUND / dev_desc->blksz, CONFIG_ENV_SIZE / dev_desc->blksz, (char *)&env);
    if (blkswrote != CONFIG_ENV_SIZE / 512) {
	printf("Failed to write environment to disk at redundant offset %d\n", CONFIG_ENV_OFFSET_REDUND);
    } else {
	rv = 0;
    }
#endif

   return rv;
}
#endif /* CMD_SAVEENV */

