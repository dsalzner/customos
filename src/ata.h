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
 * @file ata.h
 * @brief Ata Disk Driver
 *
 * Initializes, reads and writes to IDE/ATA Disks
 *
 * @see https://github.com/levex/osdev/blob/master/drivers/ata.c
 * @see https://github.com/jaytaph/CybOS/blob/master/source/kernel/drivers/ide/ata.c
 * @see https://github.com/encounter/osdev/blob/48d7c52fdbe8db63c17ad3264fa9d8c33c47af66/kernel/drivers/ata.c
*/

#pragma once
#define FF_FS_READONLY 0

#include "fatfs/fatfs_diskio.h"
#include "fatfs/fatfs_ff.h"

#define PCI_MAX_BUS 256
#define PCI_MAX_SLOT 32
#define PCI_MAX_FUNC 8
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define LO8(addr16) (uint8_t) (addr16 & 0x00FF)
#define HI8(addr16) (uint8_t)((addr16 & 0xFF00) >> 8)

// ---

#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01

#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05

#define ATA_REG_COMMAND 0x07
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_REG_STATUS 0x07
#define IDE_REG_ALTSTATUS 0x0C
#define IDE_SR_BSY     0x80
#define IDE_SR_ERR     0x01
#define IDE_SR_DRQ     0x08

#define IDE_REG_COMMAND       0x07
#define IDE_REG_CONTROL       0x0C
#define IDE_REG_HDDEVSEL      0x06
#define IDE_CONTROLLER_MAX_CHANNELS   4
#define IDE_CMD_IDENTIFY          0xEC
#define IDE_REG_STATUS        0x07
#define IDE_REG_LBA1          0x04
#define IDE_REG_LBA2          0x05

#define IDE_REG_DATA          0x00
#define IDE_IDENT_MODEL        54
#define IDE_CMD_IDENTIFY_PACKET   0xA1

#define IDE_IDENT_CAPABILITIES 98

#define IDE_REG_SECCOUNT0     0x02
#define IDE_REG_LBA0          0x03
#define IDE_REG_LBA1          0x04
#define IDE_REG_LBA2          0x05

#define IDE_REG_SECCOUNT1     0x08
#define IDE_REG_LBA3          0x09
#define IDE_REG_LBA4          0x0A
#define IDE_REG_LBA5          0x0B

#define IDE_CMD_READ_PIO          0x20
#define IDE_CMD_READ_PIO_EXT      0x24
#define IDE_CMD_WRITE_PIO_EXT     0x34
#define IDE_CMD_CACHE_FLUSH_EXT   0xEA

#define ATA_BLOCKSIZE 512

uint16_t diskPciReadWord(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
  uint32_t address = 0x80000000; // Bit 31 set
  address |= (uint32_t)bus << 16;
  address |= (uint32_t)slot << 11;
  address |= (uint32_t)func << 8;
  address |= (uint32_t)offset & 0xfc;
  outl (PCI_CONFIG_ADDRESS, address);
  uint32_t ret = inl (PCI_CONFIG_DATA);
  ret = ret >> ( (offset & 2) * 8) & 0xffff;
  return ret;
}

/**
 * @brief Get the base address of the first IDE Controller
 * for this:
 * - scan the PCI Bus for PCI cards that have IDE capabilities
 * - return the base address of the first one found
*/
uint32_t getFirstIdeControllerBaseAddress() {
  printf("[*] Scan for PCI IDE controllers\n");
  for (uint8_t bus = 0; bus != PCI_MAX_BUS; bus++) {
    for (uint8_t slot = 0; slot != PCI_MAX_SLOT; slot++) {
      for (uint8_t func = 0; func != PCI_MAX_FUNC; func++) {
        uint8_t class = diskPciReadWord(bus, slot, func, 0x0B);
        uint8_t subclass = diskPciReadWord(bus, slot, func, 0x0A);
        uint8_t vendorId = diskPciReadWord(bus, slot, func, 0x00);
        uint8_t deviceId = diskPciReadWord(bus, slot, func, 0x02);
        if (vendorId == 0x00FF) continue;
        if (deviceId == 0x0000) continue;
        printf("pci slot %d with %04x:%04x - %02x:%02x", slot, vendorId, deviceId, class, subclass);
        if(class = 0x01 && subclass == 0x01) {
            printf(" - IDE controller");

            char configSpace[256];
            for (int i=0; i!=128; i++) {
              uint16_t tmp = diskPciReadWord(bus, slot, func, i*2);
              configSpace[i*2+0] = LO8(tmp);
              configSpace[i*2+1] = HI8(tmp);
            }

            uint32_t bar[5];
            bar[0] = *((uint32_t*)&configSpace[0x10]) & 0xFFFFFFFC;
            bar[1] = *((uint32_t*)&configSpace[0x14]) & 0xFFFFFFFC;
            bar[2] = *((uint32_t*)&configSpace[0x18]) & 0xFFFFFFFC;
            bar[3] = *((uint32_t*)&configSpace[0x1C]) & 0xFFFFFFFC;
            bar[4] = *((uint32_t*)&configSpace[0x20]) & 0xFFFFFFFC;

            printf("\nIDE Controller Base-Address: %04x\n", bar[4]);
            return bar[4];
        }
        printf("\n");
      }
    }
  }

  printf("[E] ata::findFirstIdeControllerReturnBaseAdressRegister4 - not found\n");
  return 0;
}

/**
 * @brief Get the base address of the first IDE Hard Disk
 * for this:
 * - scan the drives connected to the IDE Controller
 * - print the modell information, check lba-compatibility
 * - return the base address of the first compatible hard disk found
*/
struct SDisk {
  uint16_t base; // port address
  uint16_t devCtl; // device addres
  uint16_t bmIde; // bus master ide
  uint8_t channelNr;
  uint8_t driveNr;
};

struct SDisk getFirstHardDiskBaseAddress(uint32_t ideControllerBaseAddress) {
  printf("[*] Scan for hard drives\n");

  struct SDisk disk;

  uint32_t hardDiskBaseAddress = 0;
  uint16_t ide_controller_ioports[][8] = {
    { 0x1F0, 0x3F0, 0x170, 0x370, 0x1E8, 0x3E0, 0x168, 0x360 }
  };

  uint16_t base = 0; // port address
  uint16_t devCtl = 0; // device addres
  uint16_t bmIde = 0; // bus master ide

  for(uint8_t channelNr = 0; channelNr != IDE_CONTROLLER_MAX_CHANNELS; channelNr++)  {
    base = ide_controller_ioports[0][channelNr * 2 + 0]; // note: only controller 0 supported
    devCtl = ide_controller_ioports[0][channelNr * 2 + 1] + 4;
    bmIde = ideControllerBaseAddress;

    for(uint8_t driveNr = 0; driveNr < 2; driveNr++) {
      // -- select drive (in CHS mode)
      outportb(base + IDE_REG_HDDEVSEL, 0xA0 | (driveNr << 4));

      // -- identify
      outportb(base + IDE_REG_COMMAND, IDE_CMD_IDENTIFY);

      if (inportb(base + IDE_REG_STATUS) == 0) {
        continue;  // No drive found
      }

      // -- poll until Busy flag is clear
      while (1) {
        uint8_t status = inportb(base + IDE_REG_STATUS);
        if ((status & IDE_SR_ERR)) { // ATAPI or other
          break;
        }
        if (!(status & IDE_SR_BSY) && (status & IDE_SR_DRQ)) { // ATA
          break;
        }
      }

      // -- check drive type
      uint8_t cl = inportb(base + IDE_REG_LBA1);
      uint8_t ch = inportb(base + IDE_REG_LBA2);

      if (cl == 0x14 && ch == 0xEB) {
        printf("ATAPI | ");
      }
      else if (cl == 0x00 && ch == 0x00) {
        printf("ATA   | ");
        if(disk.base == 0) {
          disk.base = base;
          disk.devCtl = devCtl;
          disk.bmIde = bmIde;
          disk.channelNr = channelNr;
          disk.driveNr = driveNr;
        }
      }
      else if (cl == 0x69 && ch == 0x96) {
        printf("ATAPI | ");
      }
      else {
        //printf("[E] unknown type  | ");
        continue;
      }
      printf("cl %04x ch %04x | ", cl, ch);

      // acknowledge?
      outportb(base + IDE_REG_COMMAND, IDE_CMD_IDENTIFY_PACKET);

      // -- get ide info
      char ideInfo[512];
      insl(base + IDE_REG_DATA, (uint32_t)&ideInfo, 128);

      // -- check lba28/48 capability
      uint16_t capabilities = *(uint16_t *)(ideInfo + IDE_IDENT_CAPABILITIES);
      if(capabilities & 0x200) {
        printf("lba28/48: yes | ");
      } else {
        printf("lba28/48: no | ");
      }

      // -- get device name
      char model[41];
      for (uint8_t i=0; i<40; i+=2) {
        if(ideInfo[IDE_IDENT_MODEL + i + 1] == ' ' && ideInfo[IDE_IDENT_MODEL + i] == ' ') {
          model[i] = '\0';
          break;
        }
        model[i] = ideInfo[IDE_IDENT_MODEL + i + 1];
        model[i+1] = ideInfo[IDE_IDENT_MODEL + i];
      }
      printf("model %s\n", &model);
    }
  }
  return disk;
}


void idePortWrite (
    uint16_t base, // port address
    uint16_t devCtl,  // device addres
    uint16_t bmIde,  // bus master ide
    uint8_t reg,
    uint8_t data
) {
  uint8_t noInt = 0;
  if (reg > 0x07 && reg < 0x0C) idePortWrite(base, devCtl, bmIde, IDE_REG_CONTROL, 0x80 | noInt);

  if (reg < 0x08) { /*printf("1,");*/ outportb (base + reg - 0x00, data); }
  else if (reg < 0x0C) { /*printf("2,");*/ outportb (base + reg - 0x06, data); }
  else if (reg < 0x0E) { /*printf("3,");*/ outportb (devCtl + reg - 0x0A, data); }
  else if (reg < 0x16) { /*printf("4,");*/ outportb (bmIde + reg - 0x0E, data); }

  if (reg > 0x07 && reg < 0x0C) idePortWrite(base, devCtl, bmIde, IDE_REG_CONTROL, noInt);
}

int ide_polling (uint32_t base) {
  int i;
  for (i=0; i<4; i++) inportb(base + IDE_REG_ALTSTATUS);
  while (inportb(base + IDE_REG_STATUS) & IDE_SR_BSY);
  return 0;
}

struct SDisk disk;
FATFS fs;

void ataInit() {
  uint32_t ideControllerBaseAddress = getFirstIdeControllerBaseAddress();
  disk = getFirstHardDiskBaseAddress(ideControllerBaseAddress);

  FRESULT res = f_mount(&fs, "", 0);
  if (res != FR_OK) {
    printf("f_mount error: %u\n", (uint32_t) res);
  }
}

void ataList(char * path) {
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
    printf("%s\n", info.fname);
  }
  f_closedir(&dir);
}

void ataGetFileContents(char * fileName, char * buffer, uint32_t bufferLen) {
  FIL file;
  FRESULT res;
  res = f_open(&file, fileName, FA_READ);
  if (res != FR_OK) {
    printf("failed to open file %s. error=%u\n", fileName, res);
  }
  uint32_t fileSize = f_size(&file);
  if(fileSize >= bufferLen) {
    printf("file is too large to read\n");
    return;
  }
  UINT br;
  res = f_read(&file, buffer, fileSize, &br);
  if (res != FR_OK) {
      printf("failed to read from file %s. error=%u\n", fileName, res);
  }
  f_close(&file);
  if (res != FR_OK) {
    fileSize = 0;
  }
  buffer[fileSize] = '\0';
}

void ataSetFileContents(char * fileName, char * buffer, uint32_t bufferLen) {
  FIL file;
  FRESULT res;
  res = f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) {
    printf("failed to open file %s. error=%u\n", fileName, res);
    return;
  }
  //uint32_t bufferLen = sizeof(buffer) / sizeof(buffer[0]) * 8; // needs to be multiple of 8
  
  for(uint32_t i = 0; i < bufferLen; i++) {
    UINT br;
    res = f_write(&file, ((const void*)buffer) + i, (UINT)1, &br);
    if (res != FR_OK) {
      printf("failed to write file %s. error=%u\n", fileName, res);
      return;
    }
  }
  f_close(&file);
}

void ataShowFileContents(char * fileName) {
  char buffer[200]; // no dynamic memory allocation, so fixed to max size for now
  ataGetFileContents(fileName, buffer, 200);
  printf("%s", buffer);
}

DSTATUS disk_initialize(__attribute__((unused)) BYTE pdrv) {
  return 0;
}

DSTATUS disk_status(__attribute__((unused)) BYTE pdrv) {
  return 0;
}

void seek(__attribute__((unused)) BYTE pdrv, DWORD sector, UINT count) {
  unsigned char lba_io[6];
  lba_io[0] = (sector & 0x000000FF) >> 0;
  lba_io[1] = (sector & 0x0000FF00) >> 8;
  lba_io[2] = (sector & 0x00FF0000) >> 16;
  lba_io[3] = (sector & 0xFF000000) >> 24;
  lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
  lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
  unsigned char head = 0; // Lower 4-bits of HDDEVSEL are not used here.

  // -- wait if busy
  while (inportb(disk.base + IDE_REG_STATUS) & IDE_SR_BSY);

  // -- select drive and set lba
  outportb(disk.base + IDE_REG_HDDEVSEL, 0xE0 | (disk.driveNr << 4) | head);

  // -- seek
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_SECCOUNT1, 0);
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_LBA3, lba_io[3]);
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_LBA4, lba_io[4]);
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_LBA5, lba_io[5]);

  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_SECCOUNT0, count);
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_LBA0, lba_io[0]);
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_LBA1, lba_io[1]);
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_LBA2, lba_io[2]);
}

DRESULT disk_read(BYTE pdrv, BYTE* buffer, DWORD sector, UINT count) {
  seek(pdrv, sector, count);

  // -- set direction
  idePortWrite (disk.base, disk.devCtl, disk.bmIde, IDE_REG_COMMAND, IDE_CMD_READ_PIO_EXT); // lba48 mode

  // -- pio read
  unsigned int words = 256;  // every ATA drive has a sector-size of 512-byte.
  char * bufptr = buffer;
  for (uint32_t i = 0; i < count; i++) {
    if (ide_polling (disk.base) != 0) {
        printf("[E] disk_read error\n");
        return 1; // error
    }
    insw (disk.base, (uint32_t)bufptr, words);
    bufptr += words*2;
  }
  return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buffer, DWORD sector, UINT count) {
  seek(pdrv, sector, count);
  // -- set direction
  idePortWrite(disk.base, disk.devCtl, disk.bmIde, IDE_REG_COMMAND, IDE_CMD_WRITE_PIO_EXT); // lba48 mode

  // -- pio write
  unsigned int words = 256;  // every ATA drive has a sector-size of 512-byte.
  char * bufptr = buffer;
  for (UINT i = 0; i < count; i++) {
    ide_polling (disk.base); //ide_polling(channel, 0);
    __asm__ volatile("rep outsw" : : "c"(words), "d"(disk.base), "S"((uint32_t)bufptr));
    bufptr += (words * 2);
  }
  idePortWrite(disk.base, disk.devCtl, disk.bmIde, IDE_REG_COMMAND, IDE_CMD_CACHE_FLUSH_EXT);
  ide_polling(disk.base);
  return RES_OK;
}

DRESULT disk_ioctl(__attribute__((unused)) BYTE pdrv, BYTE cmd, void* buff) {
  DRESULT dr = RES_ERROR;
  switch (cmd) {
    case CTRL_SYNC:
      dr = RES_OK;
      break;
    case GET_SECTOR_COUNT:
      *(DWORD*) buff = 8388608 / ATA_BLOCKSIZE; //16384;
      dr = RES_OK;
      break;
    case GET_BLOCK_SIZE:
      *(DWORD*) buff = ATA_BLOCKSIZE;
      dr = RES_OK;
      break;
  }
  return dr;
}
