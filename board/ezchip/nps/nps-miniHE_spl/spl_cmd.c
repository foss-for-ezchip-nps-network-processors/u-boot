/*
* Copyright (c) 2016, Mellanox Technologies. All rights reserved.
*
* This software is available to you under a choice of one of two
* licenses.  You may choose to be licensed under the terms of the GNU
* General Public License (GPL) Version 2, available from the file
* COPYING in the main directory of this source tree, or the
* OpenIB.org BSD license below:
*
*     Redistribution and use in source and binary forms, with or
*     without modification, are permitted provided that the following
*     conditions are met:
*
*      - Redistributions of source code must retain the above
*        copyright notice, this list of conditions and the following
*        disclaimer.
*
*      - Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following
*        disclaimer in the documentation and/or other materials
*        provided with the distribution.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "spl_cmd.h"
#include "../nps.h"

static int do_preboot_write(void)
{
	int preboot_sf_size;
	int preboot_sf_location;
	int preboot_sector_offset;
	int load_addr;
	char *preboot_offset;
	char *preboot_file_size;
	char *preboot_load_address;
	char sector_location[32];
	char spi_sector_location[32];
	char spi_sector_size[32];
	char cmdbuf[CONFIG_SYS_CBSIZE];

	preboot_offset = getenv("preboot_offs");
	preboot_file_size = getenv("filesize");
	preboot_load_address = getenv("loadaddr");

	/* Check environment variables */
	if (!preboot_offset || !preboot_file_size || !preboot_load_address) {
		printf("Environment variables are not defined:\n");
		printf("preboot_offs:%s\n",preboot_offset);
		printf("filesize:%s\n",preboot_file_size);
		printf("loadaddr:%s\n",preboot_load_address);
		goto usage;
	}

	/* Check file size */
	preboot_sf_size = simple_strtoul(preboot_file_size, NULL, 16);
	if (preboot_sf_size > PREBOOT_SIZE) {
		printf("file size is larger then %x\n",PREBOOT_SIZE);
		goto usage;
	}

	if (run_command ("sf probe 0", 0) < 0) /* sf probe 0 */
		goto usage;

	load_addr = simple_strtoul(preboot_load_address, NULL, 16);
	/* Write the sector after the preboot (64k) */
	sprintf(sector_location,"%x",load_addr + PREBOOT_SIZE);
	preboot_sf_location = simple_strtoul(preboot_offset, NULL, 16);
	/* Read from the start of the sector */
	preboot_sector_offset = preboot_sf_location - (preboot_sf_location % CONFIG_ENV_SECT_SIZE);

	sprintf(spi_sector_location,"%x",preboot_sector_offset);
	sprintf(spi_sector_size,"%x",CONFIG_ENV_SECT_SIZE);

	strcpy(cmdbuf,"sf read ");
	strcat(cmdbuf,sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_size);

	if (run_command (cmdbuf, 0) < 0) /* sf read ${load_addr}+64k ${preboot_sec_offs} ${preboot_size} */
		goto usage;

	/* initialize  the 64k of the preboot */
	memset((void*)(load_addr + PREBOOT_SIZE + PREBOOT_SIZE),0xFF,PREBOOT_SIZE);
	/* copy preboot to sector */
	memcpy((void*)(load_addr + PREBOOT_SIZE + PREBOOT_SIZE),(void*)load_addr,preboot_sf_size);

	strcpy(cmdbuf,"sf erase ");
	strcat(cmdbuf,spi_sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_size);

	if (run_command (cmdbuf, 0) < 0) /* sf erase ${preboot_offs} 1 block */
		goto usage;

	strcpy(cmdbuf,"sf write ");
	strcat(cmdbuf,sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_location);
	strcat(cmdbuf," ");
	strcat(cmdbuf,spi_sector_size);

	if (run_command (cmdbuf, 0) < 0) /* sf  write ${loadaddr} ${preboot_sec_offs} 1 block */
		goto usage;

	return 0;
usage:
	printf("Usage: uboot_write\n");
	return 1;
}

static int do_preboot_update(cmd_tbl_t *cmdtp, int flag,int argc, char * const argv[])
{
	int ret;
	char *arg = NULL;

	if (argc > 2)
		goto usage;

	if (argc == 2)
		arg = argv[1];

	ret = do_file_load("preboot_file",arg);
	if (ret)
		goto usage;

	ret = do_preboot_write();
	if (ret)
		goto usage;

	return 0;
usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	preboup, 2, 0, do_preboot_update,
	"Write preboot to SPI flash in ${preboot_offs}",
	"Usage: preboot_update [serial]"
);