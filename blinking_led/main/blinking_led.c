#include <stdint.h>

#define GPIO_OUT_W1TS_REG (*(volatile uint32_t *) 0x3FF44008)
#define GPIO_OUT_W1TC_REG (*(volatile uint32_t *) 0x3FF4400C)
#define GPIO_ENABLE_W1TS_REG (*(volatile uint32_t *) 0x3FF44020)

#define PIN 2

#define private static

private uint32_t get_count(void){
	uint32_t ccount;
	asm volatile("rsr %0, ccount" : "=r"(ccount));
	/*
	 * Xtensa Assembly
	 *
	 * rsr - Read Special Register
	 * rsr a2, [ccount] -> basically mov eax, [address]
	 * a2 - registers from a0 to a15
	 *
	 * ccount -> cycle counter known by the CPU 
	*/
	return ccount;
}

private void delay_ms(uint32_t ms) {
	uint32_t cycles_per_ms = 240000; // 240MHz by default
	uint32_t start = get_count();
	uint32_t wait = ms * cycles_per_ms;

	while ((get_count() - start) < wait);
}

void app_main(void){
	GPIO_ENABLE_W1TS_REG = (1 << PIN); // enable the GPIO 

	int sec = 10;
	while (sec >= 0){
		GPIO_OUT_W1TS_REG = (1 << PIN);
		delay_ms(500);
		GPIO_OUT_W1TC_REG = (1 << PIN);
		delay_ms(500);
		sec--;
	}
}
