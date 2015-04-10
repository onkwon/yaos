#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__

#define POINT_FACTOR	1000
#define tofixed(v)	((v) * POINT_FACTOR)
#define fromfixed(v)	((v) / POINT_FACTOR)

typedef long fixed;

static inline fixed fixed_new(int i, int p)
{
	/*
	int point, point_place = POINT_FACTOR;

	for (point = p; point; point /= 10)
		point_place /= 10;

	return (i * POINT_FACTOR) + (p * point_place);
	*/
	return i * POINT_FACTOR + p;
}

static inline fixed fixed_int(fixed f)
{
	return f / POINT_FACTOR;
}

static inline fixed fixed_point(fixed f)
{
	if (f < 0) f = -f;
	return f % POINT_FACTOR;
}

static inline fixed fixed_mul(fixed a, fixed b)
{
	return a * b / POINT_FACTOR;
}

static inline fixed fixed_div(fixed a, fixed b)
{
	return a * POINT_FACTOR / b;
}

#endif /* __FIXED_POINT_H__ */
