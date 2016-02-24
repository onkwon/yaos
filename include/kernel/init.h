#ifndef __INIT_H__
#define __INIT_H__

#define __init			__attribute__((section(".text.init"), used))

#define REGISTER_INIT(func, order) \
	static void *init_##func \
	__attribute__((section(".text.init."#order \
					",\"ax\",\%progbits @"), used)) = func

extern int kernel_init();

#endif /* __INIT_H__ */
