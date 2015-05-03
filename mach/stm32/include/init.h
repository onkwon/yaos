#ifndef __INIT_H__
#define __INIT_H__

#define __init			__attribute__((section(".text.init"), used))

#define REGISTER_INIT_FUNC(func, order) \
	static void *init_##func \
	__attribute__((section(".text.init_list."#order \
					",\"ax\",\%progbits @"), used)) = func

extern int main();

#endif /* __INIT_H__ */
