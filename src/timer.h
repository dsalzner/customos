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
 * @file timer.h
 * @brief Timer with using the Programmable Interval Timer (PIT)
 *
 * @see http://www.osdever.net/bkerndev/Docs/pit.htm
 *
*/

#define IRQ(x) x+32 // also in isrs.h

int timer_ticks = 0;

void timerPhase(int hz) {
  int divisor = 1193180 / hz;       // Calculate our divisor
  outportb(0x43, 0x36);             // Set our command byte 0x36
  outportb(0x40, divisor & 0xFF);   // Set low byte of divisor
  outportb(0x40, divisor >> 8);     // Set high byte of divisor
}

void timerInterrupt(struct regs *r) {
  UNUSED(r);
  timer_ticks++;
  if (timer_ticks % 1800 == 0) {
    printf("Ten seconds have passed\n");
  }
}

void timerInit(int hz) {
  timerPhase(hz);
  register_interrupt_handler(IRQ(0), &timerInterrupt);
}
