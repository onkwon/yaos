#include <foundation.h>
#include <kernel/task.h>
#include <kernel/timer.h>

static void test_printf()
{
	float a = -0.33947753;
	float b = 0.93957519;

	sleep(2);
	printf("%x %#x %#08x %#8x %-#8x %-#08x\n", &a, &a, &a, &a, &a, &a);
	printf("%x %#x %#06x %#6x %-#6x %-#06x\n", &a, &a, &a, &a, &a, &a);
	printf("f   {x: %f, y: %f}\n", (float)123456789, (float)123456789);
	printf("10f {x: %10f, y: %10f}\n", (float)123456789, (float)123456789);
	printf("010f{x: %010f, y: %010f}\n", (float)123456789, (float)123456789);
	printf("f   {x: %f, y: %f}\n", (float)1234, (float)1234);
	printf("8f  {x: %8f, y: %-8f}\n", (float)1234, (float)1234);
	printf("08f {x: %08f, y: %-08f}\n", (float)1234, (float)1234);
	printf("f   {x: %f, y: %f}\n", (float)-1234, (float)-1234);
	printf("8f  {x: %8f, y: %-8f}\n", (float)-1234, (float)-1234);
	printf("08f {x: %08f, y: %-08f}\n", (float)-1234, (float)-1234);
	printf(".2  {x: %.2f, y: %.2f}\n", (float)1234, (float)1234);
	printf(".2  {x: %.2f, y: %.2f}\n", (float)0, (float)0);
	printf("f   {x: %f, y: %f}\n", a, b);
	printf("8.2 {x: %8.2f, y: %8.2f}\n", a, b);
	printf("8.  {x: %8.f, y: %8.f}\n", a, b);
	printf("08f {x: %08f, y: %08f}\n", a, b);
	printf("08.2{x: %08.2f, y: %08.2f}\n", a, b);
	printf(".2  {x: %.2f, y: %.2f}\n", a, b);
	printf(".8  {x: %.8f, y: %.8f}\n", a, b);
	printf(".08 {x: %.08f, y: %.08f}\n", a, b);
	printf("-8.2{x: %-8.2f, y: %-8.2f}\n", a, b);
	printf("-8. {x: %-8.f, y: %-8.f}\n", a, b);
	printf("-08f{x: %-08f, y: %-08f}\n", a, b);
	printf("08.2{x: %-08.2f, y: %-08.2f}\n", a, b);
	printf("-.2 {x: %-.2f, y: %-.2f}\n", a, b);
	printf("-08f{x: %-08f, y: %-08f}\n", (float)1234, (float)1234);
	printf("-8f {x: %-8f, y: %-8f}\n", (float)1234, (float)1234);
	printf("-8.2{x: %-8.2f, y: %-8.2f}\n", (float)1234, (float)1234);
}
//REGISTER_TASK(test_printf, 0, DEFAULT_PRIORITY);
