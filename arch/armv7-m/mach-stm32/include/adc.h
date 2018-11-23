#ifndef __YAOS_STM32_ADC_H__
#define __YAOS_STM32_ADC_H__

enum {
	ADC1 		= 1,
	ADC2 		= 2,
	ADC_MAX,
};

enum {
	ADC_SAMPLE_CYCLE_1	= 0,
	ADC_SAMPLE_CYCLE_7	= 1,
	ADC_SAMPLE_CYCLE_13	= 2,
	ADC_SAMPLE_CYCLE_28	= 3,
	ADC_SAMPLE_CYCLE_41	= 4,
	ADC_SAMPLE_CYCLE_55	= 5,
	ADC_SAMPLE_CYCLE_71	= 6,
	ADC_SAMPLE_CYCLE_239	= 7,
	ADC_SAMPLE_CYCLE_MAX,
};

enum {
	ADC_TRIGGER_TIMER	= 0,
	ADC_TRIGGER_TRGO	= 4,
	ADC_TRIGGER_EXTI	= 6,
	ADC_TRIGGER_SOFT	= 7,
	ADC_TRIGGER_MAX,
};

void __adc_init(const int num, int trigger);
void __adc_temperature(const bool enable);
void __adc_terminate(const int num);
void __adc_sampling_set(const int num, int ch, int sample_time);
void __adc_channel_set(const int num, int ch);
int __adc_start(const int num, unsigned int timeout_ms);

#endif /* __YAOS_STM32_ADC_H__ */
