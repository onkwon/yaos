#ifndef __STM32_ADC_H__
#define __STM32_ADC_H__

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

void __adc_init(int nadc, int trigger);
void __adc_temperature_enable();
void __adc_temperature_disable();
void __adc_terminate(int nadc);
void __adc_sampling_set(int nadc, int ch, int sample_time);
void __adc_channel_set(int nadc, int ch);
int __adc_start(int nadc, unsigned int timeout_ms);

#define adc_init(adc, trig)		__adc_init(adc, trig)
#define adc_sampling_set(adc, ch, samp)	__adc_sampling_set(adc, ch, samp)
#define adc_channel_set(adc, ch)	__adc_channel_set(adc, ch)
#define adc_start(adc, timeout)		__adc_start(adc, timeout)
#define adc_convert(adc, timeout)	__adc_start(adc, timeout)

#endif /* __STM32_ADC_H__ */
