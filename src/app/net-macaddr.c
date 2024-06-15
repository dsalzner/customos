/*
CustomOS
Copyright (C) 2024 D.Salzner <mail@dennissalzner.de>

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

 * @file net-macaddr.c
 * @brief Userspace application to get the MAC-Address from the network card
 *
 * @see https://codereview.stackexchange.com/questions/30579/integer-to-hex-string-generator
 * @see GitHub: gnhnjac/osdev-graphical, drivers/rtl8139.c
*/

#define PCI_MAX_BUS 256
#define PCI_MAX_SLOT 32
#define PCI_MAX_FUNC 8
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

void* (*putchar)(char);
void setPutCharCallback(void* (*putchar_)(char)) {
  putchar = putchar_;
}

const char * name() {
  return "Network Card MAC address";
}

// -- write string to screen
unsigned int strlen_(const char *str) {
  for(unsigned int i = 0; i < 100; i++) { // cap at 100
    if(str[i] == '\0') {
      return i;
    }
  }
  return 0;
}

void putstr(const char *str) {
  int i = 0;
  for(i = 0; i < strlen_(str); i++) {
    putchar(str[i]);
  }
}

// -- write integers to screen
int log10_(unsigned int number) {
  unsigned int i = 0;
  unsigned int j = 1;
  for(i = 0; i < 5; i++) {
    j *= 10;
    if (number < j) {
      return i;
    }
  }
  return 0;
}

void itoa(unsigned int number, char * str) {
  if(number == 0) {
    str[0] = '0';
    str[1] = '\0';
    return;
  }
  int i = 0;
  int digits = log10_(number);
  while(number > 0) {
    str[digits - i] = number % 10 + '0';
    number = number / 10;
    i++;
  }
  str[i] = '\0';
}

void putint(uint32_t number) {
  char s[10];
  itoa(number, s);
  putstr(s);
}

// -- write hex values to screen
unsigned int num_hex_digits(unsigned n) {
  if (!n) return 1;
  int ret = 0;
  for (; n; n >>= 4) {
      ++ret;
  }
  return ret;
}
void make_hex_string(unsigned n, char *s) {
  char hex_lookup[] = "0123456789abcdef";
  int len = num_hex_digits(n);
  if (len & 1) {
    *s++ = '0';
  }
  s[len] = '\0';
  for (--len; len >= 0; n >>= 4, --len) {
    s[len] = hex_lookup[n & 0xf];
  }
}

void puthex(unsigned int value) {
  char s[10];
  make_hex_string(value, s);
  putstr(s);
}

// -- read values from PCI bus
uint32_t inl(uint32_t addr) {
  uint32_t l;
  __asm__ __volatile__ ("inl  %%dx, %%eax" : "=a" (l) : "d" (addr) );
  return l;
}

void outl(uint32_t addr, uint32_t l) {
  __asm__ __volatile__ ("outl  %%eax, %%dx" : : "d" (addr), "a" (l) );
}

uint16_t pciReadWord(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
  uint32_t address = 0x80000000; // bit 31 set
  address |= (uint32_t)bus << 16;
  address |= (uint32_t)slot << 11;
  address |= (uint32_t)func << 8;
  address |= (uint32_t)offset & 0xfc;
  outl(PCI_CONFIG_ADDRESS, address);
  uint32_t ret = inl(PCI_CONFIG_DATA);
  ret = ret >> ( (offset & 2) * 8) & 0xffff;
  return ret;
}

// -- get network card PCI slot
uint16_t getNetworkCardPciSlot() {
  for (uint8_t bus = 0; bus != PCI_MAX_BUS - 1; bus++) {
    for (uint8_t slot = 0; slot != PCI_MAX_SLOT - 1; slot++) {
      for (uint8_t func = 0; func != PCI_MAX_FUNC - 1; func++) {
	uint8_t classId = pciReadWord(bus, slot, func, 0x0B);
	uint8_t subclassId = pciReadWord(bus, slot, func, 0x0A);
	uint16_t vendorId = pciReadWord(bus, slot, func, 0x00);
	uint16_t deviceId = pciReadWord(bus, slot, func, 0x02);
	if((vendorId = 0x10EC) && (deviceId == 0x8139)) {
	  return slot;
	}
      }
    }
  }
  return 0;
}

// --

void outportb(uint16_t port, uint8_t val) {
  asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inportb(uint16_t port) {
  uint8_t ret;
  asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// -- gnhnjac/osdev-graphical
uint32_t pci_config_read_long(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  uint32_t address; // register offset
  uint32_t lbus  = (uint32_t)bus; // pci bus number
  uint32_t lslot = (uint32_t)slot; // device number
  uint32_t lfunc = (uint32_t)func; // device function

  // - first 2 bits 0 for byte aligned
  // - followed by bus no, device no, function no, register offset
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

  outl(PCI_CONFIG_ADDRESS, address);
  return inl(PCI_CONFIG_DATA);
}
// --

void init() {
  // -- get pci slot
  uint16_t slot = getNetworkCardPciSlot();
  char s2[] = "Network controller, rtl8139 on slot: "; putstr(s2); putint(slot); putchar('\n');

  // -- get io base address
  uint32_t pci_bar_0 = pci_config_read_long(0,slot,0,0x10);
  uint16_t io_base = pci_bar_0 & (~0x3);

  // -- get mac address
  uint8_t mac_addr[6];
  for (uint8_t i = 0; i < 6; i++) {
	    mac_addr[i] = inportb(io_base + i);
	    puthex(mac_addr[i]);
	    if(i != 5) putchar(':');
	}
}
