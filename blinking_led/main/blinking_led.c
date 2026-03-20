#include <stdint.h>
#include <stdio.h>
#include "xtensa_api.h"

// Define the GPIO HIGH 
#define GPIO_OUT_W1TS_REG (*(volatile uint32_t *) 0x3FF44008)
// Define the GPIO LOW
#define GPIO_OUT_W1TC_REG (*(volatile uint32_t *) 0x3FF4400C)
// Define the GPIO EN address
#define GPIO_ENABLE_W1TS_REG (*(volatile uint32_t *) 0x3FF44020)

// Define the configures of Time Group 
#define TIMG0_T0CONFIG_REG (*(volatile uint32_t *) 0x3FF5F000)
// Alarm value register
#define TIMG_T0ALARM_LO_REG (*(volatile uint32_t *) 0x3FF5F010)
#define TIMG_T0ALARM_HI_REG (*(volatile uint32_t *) 0x3FF5F014)
// Timer 0 interrupt and Flags
#define TIMG_INT_ENA_REG (*(volatile uint32_t *) 0x3FF5F098)
#define TIMG_INT_CLR_REG (*(volatile uint32_t *) 0x3FF5F0A4)
#define TIMG_INT_ST_REG (*(volatile uint32_t *) 0x3FF5F09C)
// Reload Timers
#define TIMG0_T0LOAD_HI_REG (*(volatile uint32_t *) 0x3FF5F01C)
#define TIMG0_T0LOAD_LO_REG (*(volatile uint32_t *) 0x3FF5F018)
#define TIMG0_T0LOAD_REG (*(volatile uint32_t *) 0x3FF5F020)

// Define the Interrupt Matrix register for Time Group to route the peripheral interrupt
#define DPORT_PRO_TG_T0_LEVEL_INT_MAP_REG (*(volatile uint32_t *) 0x3FF0013C)

//  T0CONFIG bit positions
//  Bit 31: T0_EN           (enable timer)
//  Bit 30: T0_INCREASE     (1 = count up)
//  Bit 29: T0_AUTORELOAD   (reload on alarm)
//  Bits 28:13: T0_DIVIDER  (16-bit clock divider)
//  Bit 11: T0_LEVEL_INT_EN (enable level interrupt)
//  Bit 10: T0_ALARM_EN     (enable alarm)
#define T0_EN          		(1 << 31)
#define T0_INCREASE     	(1 << 30)
#define T0_AUTORELOAD   	(1 << 29)
#define T0_DIVIDER_SHIFT 	13
#define T0_DIVIDER_MASK     	(0xFFFF << T0_DIVIDER_SHIFT)
#define T0_LEVEL_INT_EN     	(1 << 11)
#define T0_ALARM_EN         	(1 << 10)

// APB clock = 80 MHz, divider = 80 => timer clock = 1 MHz (1 tick = 1 microsecond)
// Alarm at 500000 => interrupt every 500 ms => LED toggles at 1 Hz
#define TIMER_DIVIDER       80
#define ALARM_VALUE         500000

#define PIN 27
#define CPU_INT_LINE 5

#define private static

private volatile uint32_t led_state = 0;
private volatile uint32_t isr_count = 0;

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

private inline void clear_cpu_interrupt(uint32_t line){
	asm volatile("wsr %0, intclear" :: "r"(1 << line));
}


private void timer_isr(void *arg) {
	// Clear the timer interrupt flag in the Timer Group
	TIMG_INT_CLR_REG = (1 << 0);
	// Clear the CPU side for safety too
	clear_cpu_interrupt(CPU_INT_LINE);

	isr_count++;

	if (led_state) {
        	GPIO_OUT_W1TC_REG = (1 << PIN);   // LOW
        	led_state = 0;
    	} else {
        	GPIO_OUT_W1TS_REG = (1 << PIN);   // HIGH
        	led_state = 1;
    	}

	// Re-enable alarm
    	TIMG0_T0CONFIG_REG |= T0_ALARM_EN;

	// Re-enable timer interrupt in Timer Group
    	TIMG_INT_ENA_REG |= (1 << 0);
}

//  When CPU interrupt line fires, it jumps to the corresponding interrupt vector. We hook level 6 by providing a
//  handler that saves minimal context, calls our C ISR, restores context, and returns via 'rfi'
// 	This replaces esp_intr_alloc()

private void timer_init(void) {
    printf("timer_init: stopping timer\n");
    TIMG0_T0CONFIG_REG &= ~T0_EN;
    
    printf("timer_init: setting divider\n");
    TIMG0_T0CONFIG_REG &= ~T0_DIVIDER_MASK;
    TIMG0_T0CONFIG_REG |= (TIMER_DIVIDER << T0_DIVIDER_SHIFT);
    TIMG0_T0CONFIG_REG |= T0_INCREASE;
    TIMG0_T0CONFIG_REG |= T0_AUTORELOAD;
    TIMG0_T0CONFIG_REG |= T0_LEVEL_INT_EN;
    TIMG0_T0CONFIG_REG |= T0_ALARM_EN;

    printf("timer_init: loading counter\n");
    TIMG0_T0LOAD_HI_REG = 0;
    TIMG0_T0LOAD_LO_REG = 0;
    TIMG0_T0LOAD_REG    = 1;

    printf("timer_init: setting alarm\n");
    TIMG_T0ALARM_HI_REG = 0;
    TIMG_T0ALARM_LO_REG = ALARM_VALUE;

    printf("timer_init: enabling interrupts\n");
    TIMG_INT_ENA_REG |= (1 << 0);
    DPORT_PRO_TG_T0_LEVEL_INT_MAP_REG = CPU_INT_LINE;

    printf("timer_init: registering handler\n");
    xt_set_interrupt_handler(CPU_INT_LINE, timer_isr, NULL);

    printf("timer_init: enabling CPU interrupt\n");
    enable_cpu_interrupt(CPU_INT_LINE);

    printf("timer_init: starting timer\n");
    TIMG0_T0CONFIG_REG |= T0_EN;
    
    printf("timer_init: done\n");
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

private void __attribute__((unused)) delay_ms(uint32_t ms) {
    uint32_t cycles_per_ms = 240000;
    uint32_t start = get_count();
    uint32_t wait = ms * cycles_per_ms;
    while ((get_count() - start) < wait);
}

void app_main(void) {
    	// Enable GPIO pin as output
    	GPIO_ENABLE_W1TS_REG = (1 << PIN);
	
	// manual test
	delay_ms(3000);
	GPIO_OUT_W1TC_REG = (1 << PIN);    // OFF
	delay_ms(1000);
   	GPIO_OUT_W1TS_REG = (1 << PIN);   // ON
    	delay_ms(1000);
    	GPIO_OUT_W1TC_REG = (1 << PIN);   // OFF
    	delay_ms(1000);
    	GPIO_OUT_W1TS_REG = (1 << PIN);   // ON
    	delay_ms(1000);
    	GPIO_OUT_W1TC_REG = (1 << PIN);   // OFF
				
	timer_init();

    	while (1) {
        	// CPU is free — LED toggling is handled purely by the timer ISR
        	asm volatile("nop");
    	}
}
