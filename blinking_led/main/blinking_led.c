#include <stdint.h>

// Define the GPIO HIGH 
#define GPIO_OUT_W1TS_REG (*(volatile uint32_t *) 0x3FF44008)
// Define the GPIO LOW
#define GPIO_OUT_W1TC_REG (*(volatile uint32_t *) 0x3FF4400C)
// Define the GPIO EN address
#define GPIO_ENABLE_W1TS_REG (*(volatile uint32_t *) 0x3FF44020)

// Define the configures of Time Group 
#define TIMG0_T0CONFIG_REG (*(volatile uint32_t *) 0x3FF5F000)
// Define the Interrupt Matrix register for Time Group to route the peripheral interrupt
#define DPORT_PRO_TG_T0_LEVEL_INT_MAP_REG (*(volatile uint32_t *) 0x3FF0013C)
// Alarm value register
#define TIMG_T0ALARM_LO_REG (*(volatile uint32_t *) 0x3FF5F010)
// Timer 0 interrupt
#define TIMG_INT_ENA_REG (*(volatile uint32_t *) 0x3FF5F098)

// Reload Timers
#define TIMG0_T0LOAD_HI_REG (*(volatile uint32_t *) 0x3FF5F01C)
#define TIMG0_T0LOAD_LO_REG (*(volatile uint32_t *) 0x3FF5F018)
#define TIMG0_T0LOAD_REG (*(volatile uint32_t *) 0x3FF5F020)

#define PIN 2

#define private static

private inline void enable_cpu_interrupt(uint32_t line) {
    uint32_t intenable;
    asm volatile("rsr %0, intenable" : "=r"(intenable));
    intenable |= (1 << line);
    asm volatile("wsr %0, intenable" :: "r"(intenable));
    /*
     * 	rsr a2, intenable       ; store the INTENABLE value to a2 (read)	
     * 	movi a3, 1              ; a3 = 1
     * 	slli a3, a3, 1          ; a3 = 1 << 1
     * 	or a2, a2, a3           ; a2 = a2 | a3
     * 	wsr a2, intenable       ; write back to INTENABLE
	*/
}

private inline uint32_t get_count(void) {
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

private void timer_init() {
	DPORT_PRO_TG_T0_LEVEL_INT_MAP_REG = 1;	// route to CPU Interrupt line 1
	
	// Stop Timer
	TIMG0_T0CONFIG_REG &= ~(1 << 31);

	// Set divider to 80
	TIMG0_T0CONFIG_REG |= (80 << 13);

	// Enable Autoreload
	TIMG0_T0CONFIG_REG |= (1 << 29);

	// Enable Alarm
	TIMG0_T0CONFIG_REG |= (1 << 10);
	
	// load counter = 1
	TIMG0_T0LOAD_HI_REG = 0;
	TIMG0_T0LOAD_LO_REG = 0;
	TIMG0_T0LOAD_REG = 1;

	// 1000 at 1MHz = 1ms
	TIMG_T0ALARM_LO_REG = 1000;
	
	// Enable Time Interrupt in Time Group
	TIMG_INT_ENA_REG |= (1 << 0);

	enable_cpu_interrupt(1);

	// Enable Timer
	TIMG0_T0CONFIG_REG |= (1 << 31);
}

void app_main(void) {
	GPIO_ENABLE_W1TS_REG = (1 << PIN); // enable the GPIO 

	timer_init();

	while (1){
		GPIO_OUT_W1TS_REG = (1 << PIN);
		delay_ms(500);
		GPIO_OUT_W1TC_REG = (1 << PIN);
		delay_ms(500);
	}
}
