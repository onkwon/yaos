#ifndef __MOCK_H__
#define __MOCK_H__

char _ram_end;
long _ram_start;
long _rom_start;

void __nop(void) {}
void __dmb(void) {}
void __dsb(void) {}
void __isb(void) {}
void __ret(void) {}

void ISR_reset(void) {}

#endif /* __MOCK_H__ */
