#ifndef __YAOS_INIT_H__
#define __YAOS_INIT_H__

#if !defined(TEST)
#define __init			__attribute__((section(".text.init"), used))

#define REGISTER_INIT(func, order) \
	static void (*init_##func)(void) \
	__attribute__((section(".text.init."#order \
					",\"ax\",%progbits @"), used)) = func
#else
#define __init
#define REGISTER_INIT(func, order)
#endif

void kernel_init(void);

#endif /* __YAOS_INIT_H__ */
