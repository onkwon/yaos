#ifndef __RPI_SYSTICK_H__
#define __RPI_SYSTICK_H__

unsigned int getclk();
unsigned int get_sysclk_freq();

void run_sysclk();
void stop_sysclk();
unsigned int get_sysclk();
unsigned int get_sysclk_max();

int sysclk_init();

#endif /* __RPI_SYSTICK_H__ */
