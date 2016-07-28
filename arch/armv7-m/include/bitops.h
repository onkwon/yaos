#ifndef __ARMv7M_BITOPS_H__
#define __ARMv7M_BITOPS_H__

/* count leading zeros */
static inline unsigned int __clz(unsigned int v)
{
	unsigned int ret;

	__asm__ __volatile__("clz %0, %1": "=r"(ret): "r"(v));

	return ret;
}

#endif /* __ARMv7M_BITOPS_H__ */
