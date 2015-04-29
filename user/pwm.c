
#include "pwm.h"
int32_t PWM_FACTOR = 0x10;

#define DIVDED_BY_1 0
#define DIVDED_BY_16 4
#define DIVDED_BY_256 8
#define DIVDE_BIT DIVDED_BY_256
//APB_CLK_FREQ>>DIVDE_BIT

#define TM_EDGE_INT 0
#define TM_LEVEL_INT 1
#define FRC1_ENABLE_TIMER 0x80

//APB_CLK_FREQ>>DIVDE_BIT
static bool pwmInit = true;

uint8_t analogValue[16] = {0};
uint8_t pwmValue = 0;

void ICACHE_RAM_ATTR pwm_timer_intr_handler() {
//	os_printf("pwm_timer_intr_handler");
	
	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
	if (pwmValue == PWM_DEPTH) pwmValue = 0;
	
	uint16_t on = 0, off = 0;
	uint8_t v = PWM_DEPTH;
	
	for (register int i = 0; i < 16; i++) {
		//if (!pwmEnabled[i]) continue;
		register uint8_t a = analogValue[i];
		if (!a) continue;
		if (pwmValue < a) {
			if (a < v) v = a;
			on |= 0x1 << i;
		}
		else {
			off |= 0x1 << i;
		}
	}
	gpio_output_set(on, off, 0, 0);
//	os_printf(" %d %d\n", on, off);
// analogWrite(13, 100);
	register uint8_t t = v - pwmValue;
	if (t == PWM_DEPTH) {
		pwmValue = 0;
		return;
	}

	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, t * PWM_FACTOR);
	pwmValue = v;
}

void pwm_init() {
	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();

	pwmInit = false;
	RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
				  DIVDE_BIT
				  | FRC1_ENABLE_TIMER
				  | TM_EDGE_INT);
	
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, PWM_DEPTH * PWM_FACTOR);
	ETS_FRC_TIMER1_INTR_ATTACH(pwm_timer_intr_handler, NULL);

	os_printf("pwm_init\n");
}

void ICACHE_RAM_ATTR pwm_set(int pin, uint8_t value) {
	if (pwmInit) pwm_init();
	
	if (0 == value) { // digital off
		analogValue[pin] = 0;
		gpio_output_set(0, 0x1 << pin, 0, 0);
		return;
	}
	if (PWM_DEPTH == value) { // digital on
		analogValue[pin] = 0;
		gpio_output_set(0x1 << pin, 0, 0, 0);
		return;
	}
	// use pwm
	analogValue[pin] = value;
	if (0 == pwmValue) { // run pwm
		pwmValue = value;
		gpio_output_set(0x1 << pin, 0, 0, 0); // current on
		RTC_REG_WRITE(FRC1_LOAD_ADDRESS, value * PWM_FACTOR);
	}
}
