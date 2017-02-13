#include <foundation.h>
#include <kernel/task.h>

#define TIM_COUNTER_MAX		(1 << 16) /* 65536 */

enum timer_mode {
	O_TIM_OUTPUT, /* overflow/underflow : default */
	O_TIM_OUTCOMP,
	O_TIM_OUTCOMP_PWM,
	O_TIM_INPUT,
	O_TIM_INPUT_PWM,
	O_TIM_MAX,
};

static unsigned int calc_arr(unsigned int fclk, unsigned int hz)
{
	/* TODO: Do not use float data type but int */
	float quotient, remainder, min;
	unsigned int arr, reserve;

	if (fclk < hz)
		return 0;

	min = TIM_COUNTER_MAX;

	/* TODO: change the way of finding the largest value of arr */
	for (arr = TIM_COUNTER_MAX; arr; arr--) {
		quotient = (float)fclk / hz / arr;
		remainder = quotient - (int)quotient;

		if (remainder < min) {
			min = remainder;
			reserve = arr;
		}

		if (remainder <= 0)
			break;
	}

	if (!arr)
		arr = reserve;

	return arr;
}

static unsigned int calc_psc(unsigned int fclk, unsigned int hz,
		unsigned int arr)
{
	if (fclk < hz)
		return 0;

	return fclk / (hz * arr);
}

#define COUNTER_EN		0
#define CMS	5
#define DIR	4
#define ARPE	7

#define OVERCAPTURE_MASK	0x1e00
#define COMPARE_MASK		0x1e
#define TIF			6
#define UPDATE_INT			0

#include "pcm.h"

static void isr_tim2()
{
	int flags = TIM2_SR;
	static int i = 0;

	if (flags & OVERCAPTURE_MASK)
		warn("overcapture: the value captured overwritten");

	if (flags & COMPARE_MASK) {
	}

	if (flags & (1 << UPDATE_INT)) {
		//notice("TIM2: update interrupt");
		TIM3_CCR1 = pcm[i];
		if (++i >= pcm_len)
			i = 0;
	}

	TIM2_SR = 0;
}

static void isr_tim3()
{
	int flags = TIM3_SR;

	if (flags & OVERCAPTURE_MASK)
		warn("overcapture: the value captured overwritten");

	if (flags & COMPARE_MASK) {
		//notice("Compare INT %x\n", flags);
	}

	if (flags & (1 << UPDATE_INT)) {
		//notice("TIM3: update interrupt");
	}

	TIM3_SR = 0;
}

static void t_init()
{
	// peripheral reset
	SET_CLOCK_APB1(ENABLE, 0);
	RCC_APB1RSTR |= (1 << 0);
	RCC_APB1RSTR &= ~(1 << 0);

	// prescaler, TIM2_PSC
	// direction, TIM2_SMCR->DIR
	// auto-reload, TIM2_ARR
	//
	// default
	TIM2_CR1 = (0 << CMS) | (0 << DIR); /* edge aligned, upcounter */
	//TIM2_CCER = (0 << CCxP) /* active high or non-inverted */
	//	| (0 << CCxE); /* output pin, OC1 not activated */
#if 0
#if O_TIM_OUTCOMP
	;
#elif O_TIM_OUTCOMP_PWM
	TIM2_CR |= (1 << ARPE);
#elif O_TIM_INPUT
	;
#elif O_TIM_INPUT_PWM
	;
#endif
#endif
	int nvector = 44;
	register_isr(nvector, isr_tim2);
	nvic_set(vec2irq(nvector), true);

	TIM2_ARR = calc_arr(get_hclk(), 16000);
	TIM2_PSC = calc_psc(get_hclk(), 16000, TIM2_ARR);
	TIM2_DIER = (1 << UPDATE_INT);
	TIM2_CR1 |= (1 << COUNTER_EN);
}

static void t2_init()
{
	SET_CLOCK_APB1(ENABLE, 1);
	RCC_APB1RSTR |= (1 << 1);
	RCC_APB1RSTR &= ~(1 << 1);
	gpio_init(6, GPIO_MODE_ALT | GPIO_MODE_OUTPUT);

	int nvector = 45;
	register_isr(nvector, isr_tim3);
	nvic_set(vec2irq(nvector), true);

	TIM3_CR1 |= (1 << ARPE);
	TIM3_CCER |= (0 << 1) | (1 << 0); // CC1P, CC1E : PWM mode 1
	TIM3_CCMR1 = (6 << 4) | (1 << 3); // OCM | PRE
	TIM3_CCR1 = 0;

	//TIM3_ARR = calc_arr(get_hclk(), 1);
	//TIM3_PSC = calc_psc(get_hclk(), 1, TIM3_ARR);
	TIM3_ARR = 255;
	//TIM3_PSC = 34;
	//TIM3_PSC = 8;

	TIM3_EGR = 1;
	//TIM3_DIER = (1 << UPDATE_INT) | (1 << 1);
	TIM3_DIER = (1 << 1);
	TIM3_CR1 |= (1 << COUNTER_EN);
}

static int t_open(int mode)
{
	if (mode >= O_TIM_MAX)
		return -1;

	switch (mode) {
	case O_TIM_OUTPUT:
	case O_TIM_OUTCOMP:
		//1. 클럭소스 설정
		//2. CCR 및 ARR 설정
		//3. 인터럽트 및 DMA 설정
		//4. 출력 모드 설정, e.g. 출력핀을 토글하려면, OCxM=011, OCxPE=0, CCxP=0, CCxE=1
		//5. 카운터 활성화
	case O_TIM_OUTCOMP_PWM:
		break;
	case O_TIM_INPUT:
		//1. 입력 설정, `TIM_CCMR1->CCxS`
		//2. 입력필터 설정, `TIM_CCMR->ICxF`
		//3. edge 설정, `TIM_CCER->CCxP`
		//4. 프리스케일러 설정, `TIM_CCMR1->ICxPS`
		//5. 캡처 활성화, `TIM_CCER->CCxE`
		//6. 경우에 따라 인터럽트 또는 DMA 활성화, `TIM_DIER->CCxIE`, `TIM_DIER->CCxDE`
		break;
	case O_TIM_INPUT_PWM:
		//1. 입력1 설정, `TIM_CCMR1->CC1S`
		//2. 입력1에 대한 edge 설정, `TIM_CCER->CC1P`
		//3. 입력2 설정, `TIM_CCMR1->CC2S`
		//4. 입력2에 대한 edge 설정, `TIM_CCER->CC2P`
		//5. 트리거 신호 설정, `TIM_SMCR->TS`
		//6. 슬레이브 모드 컨트롤러를 리셋모드로, `TIM_SMCR->SMS`
		//7. 캡처 활성화, `TIM_CCER->CC1E/CC2E`
		break;
	default:
		break;
	}

	return 0;
}

static void pcm_test()
{
	t_init();
	t2_init();

	unsigned int hz = 1, arr;

	while (1);
	while (1) {
#if 1
		arr = calc_arr(72000000, hz);
		printf("=> %d: %d %d\n", hz, calc_psc(72000000, hz, arr), arr);
		hz++;
#else
		printf("%d\n", TIM2_CNT);
		sleep(1);
#endif
	}
}
REGISTER_TASK(pcm_test, TASK_KERNEL, DEFAULT_PRIORITY);
