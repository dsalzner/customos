/*
CustomOS
Copyright (C) 2023 D.Salzner <mail@dennissalzner.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file ramdisk.h
 * @brief Ramdisk for Custom OS
 *
 * Single-Header for intializing the Ramdisk
 * - implements interface from fatfs/fatfs_diskio.h
*/

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "fatfs/fatfs_diskio.h"
#include "fatfs/fatfs_ff.h"
#include "common.h"
#include "printf.h"
#include "multiboot.h"


#define RAMDISK_BLOCKSIZE 512

static struct {
  uint32_t location;
  uint32_t size;
  FATFS fs;
} ramdisk;

void _loadRamdisk(uint32_t location, uint32_t size) {
  printf("[ ] Ramdisk - Found at %x with %u bytes\n", location, size);
  ramdisk.location = location;
  ramdisk.size = size;
  FRESULT res;
  res = f_mount(&ramdisk.fs, "", 0);
  if (res != FR_OK) {
    printf("f_mount error: %u\n", (uint32_t) res);
  }
}

void ramdiskInit(struct multiboot_info* info) {
  uint32_t mod_count = info->mods_count;
  if(mod_count == 0) {
    printf("RamDisk Module not loaded\n");
  }
  uint32_t mod0 = *(uint32_t*) (info->mods_addr);
  uint32_t mod1 = *(uint32_t*) (info->mods_addr + 4);
  uint32_t size = mod1 - mod0;

  printf("module addr: %x\n", mod0);
  printf("module size: %x\n", size);

  _loadRamdisk(mod0 + 0xC0000000, size);
}

void ramdiskList(char * path) {
  FRESULT res;
  DIR dir;
  res = f_opendir(&dir, path);
  if(res != FR_OK) {
    printf("error f_opendir\n");
  }
  FILINFO info;
  while (true) {
    res = f_readdir(&dir, &info);
    if(res != FR_OK) {
      printf("error f_readdir\n");
    }
    if (info.fname[0] == 0)
      break;
    printf("- %s\n", info.fname);
  }
  f_closedir(&dir);
}

DSTATUS disk_initialize(__attribute__((unused)) BYTE pdrv) {
  return 0;
}

DSTATUS disk_status(__attribute__((unused)) BYTE pdrv) {
  return 0;
}

DRESULT disk_read(__attribute__((unused)) BYTE pdrv, BYTE* buffer, DWORD sector, UINT count) {
  uint32_t offset = sector * RAMDISK_BLOCKSIZE;
  uint32_t size = count * RAMDISK_BLOCKSIZE;
  memcpy(buffer, (uint8_t*) (ramdisk.location + offset), size);
  return RES_OK;
}

DRESULT disk_ioctl(__attribute__((unused)) BYTE pdrv, BYTE cmd, void* buff) {
  DRESULT dr = RES_ERROR;
  switch (cmd) {
  case CTRL_SYNC:
      dr = RES_OK;
      break;
  case GET_SECTOR_COUNT:
      *(DWORD*) buff = ramdisk.size / RAMDISK_BLOCKSIZE;
      dr = RES_OK;
      break;
  case GET_BLOCK_SIZE:
      *(DWORD*) buff = RAMDISK_BLOCKSIZE;
      dr = RES_OK;
      break;
  }
  return dr;
}
