#ifndef __YAOS_INIT_H__
#define __YAOS_INIT_H__

#if !defined(TEST)
#define __init			__attribute__((section(".text.init"), used))

#define REGISTER_INIT(func, order)				\
	static void (*init_##func)(void)			\
	__attribute__((section(".text.init."#order		\
					",\"ax\",%progbits @"), used)) = func
#define DRIVER_INIT(func)					\
	static void *module_##func				\
	__attribute__((section(".driver_list"), used)) = func
#else
#define __init
#define REGISTER_INIT(func, order)
#define DRIVER_INIT(func)
#endif

void kernel_init(void);
void freeze(void);

#endif /* __YAOS_INIT_H__ */
