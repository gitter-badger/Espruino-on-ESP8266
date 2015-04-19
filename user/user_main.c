
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "stdout.h"
#include "uart.h"
#include "user_interface.h"

#include "platform_config.h"
#include "jsinteractive.h"
#include "jshardware.h"

#include "jsvar.h"
#include "jswrap_functions.h"
#include "spi_flash.h"
#include "jswrap_math.h"
#include "pwm.h"

// error handler for pure virtual calls
void __cxa_pure_virtual() { while (1); }

void jsInit(bool autoLoad) {
	jshInit();
	jsvInit();
	jsiInit(autoLoad);
}

void jsKill() {
	jsiKill();
	jsvKill();
	jshKill();
}

#define malloc os_malloc
#define free os_free

const char *ICACHE_RAM_ATTR jsVarToString(JsVar *jsVar) {
	if (!jsVar) return "undefined";
	jsVar = jsvAsString(jsVar, true/*unlock*/);
	size_t len = jsvGetStringLength(jsVar);
	if (0 == len) return "''";
	static char *str = NULL;
	static size_t size = 0;
	if (!str) str = malloc(size = len+32);
	else if (size < len+1) {
		free(str);
		str = malloc(size = len+32);
	}
	len = jsvGetString(jsVar, str, size);
	str[len] = 0;
	return str ? str : "undefined";
}

extern UartDevice UartDev;

void writeToFlash(JsVar *jsCode) {
	if (!jsCode) return;
	const char *code = jsVarToString(jsCode);
	int error;
	int addr = 0x60000;
	int sector = addr/SPI_FLASH_SEC_SIZE;
	int to = addr + strlen(code)+1;
	while (addr < to) {
		spi_flash_erase_sector(sector);
		if (SPI_FLASH_RESULT_OK != (error = spi_flash_write(addr, (uint32 *)code, SPI_FLASH_SEC_SIZE))) {
			jsiConsolePrintf("\nwriteToFlash error %d\n", error);
		}
		addr += SPI_FLASH_SEC_SIZE;
		code += SPI_FLASH_SEC_SIZE;
		sector++;
	}
}

void printTime(JsSysTime time) {
	JsVarFloat ms = jshGetMillisecondsFromTime(time);
	jsiConsolePrintf("time: %d, %f, %d\n", (int)time, ms, (int)jshGetTimeFromMilliseconds(ms));
}
void printMilliseconds(JsVarFloat ms) {
	JsSysTime time = jshGetTimeFromMilliseconds(ms);
	jsiConsolePrintf("ms: %f, %d, %f\n", ms, (int)time, jshGetMillisecondsFromTime(time));
}

void test() {
	JsSysTime time = 1;
	for (int n = 0; n < 15; n++) {
		printTime(time);
		printTime(time/10);
		time *= 10;
	}
	JsVarFloat ms = 1.0;
	for (int n = 0; n < 15; n++) {
		printMilliseconds(ms);
		printMilliseconds(ms/10.0);
		ms *= 10.0;
	}
}

#include "jswrapper.h"
void addNativeFunction(const char *name, void (*callbackPtr)(void)) {
	jsvUnLock(jsvObjectSetChild(execInfo.root, name, jsvNewNativeFunction(callbackPtr, JSWAT_VOID)));
}

void nativeSave() {
	jsiConsolePrintf("nativeSave\n");
}

void ICACHE_RAM_ATTR user_init(void) {
	uart_init(BIT_RATE_115200, 0);
	
	jshPinSetState(2, JSHPINSTATE_GPIO_OUT);
	jshPinSetState(12, JSHPINSTATE_GPIO_OUT);
	jshPinSetState(13, JSHPINSTATE_GPIO_OUT);
	jshPinSetState(15, JSHPINSTATE_GPIO_OUT);

	jshPinSetValue(2, false);
	jshPinSetValue(12, false);
	jshPinSetValue(13, false);
	jshPinSetValue(15, false);
	
	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();
	int c = 0;
	uint8_t r = 0, g = 0, b = 0;
	
	while (true) {
		int phase = c/PWM_DEPTH;
		int value = c - phase * PWM_DEPTH;
		switch (phase) {
			case 0: // 1 R 0
				r = PWM_DEPTH;
				g = value;
				b = 0;
				break;
			case 1: // F 1 0
				r = PWM_DEPTH - value;
				g = PWM_DEPTH;
				b = 0;
				break;
			case 2: // 0 1 R
				r = 0;
				g = PWM_DEPTH;
				b = value;
				break;
			case 3: // 0 F 1
				r = 0;
				g = PWM_DEPTH - value;
				b = PWM_DEPTH;
				break;
			case 4: // R 0 1
				r = value;
				g = 0;
				b = PWM_DEPTH;
				break;
			case 5: // 1 0 F
				r = PWM_DEPTH;
				g = 0;
				b = PWM_DEPTH - value;
				break;
		}

		g /= 2; // green is 2x stronger than 2 other colors
		uint16_t r16 = (uint16_t)(r) * 2;
		if (r16 > PWM_DEPTH) r = PWM_DEPTH;
		else r = (uint8_t)r16;
		
		int p = c/128;
		int v = c - p * 128;
		p = p % 2;
		if (p) v = 128-v;
		float f = 1.0f - v / 128.0f;
		pwm_set(15, r * f);
		pwm_set(12, g * f);
		pwm_set(13, b * f);
		
		jshDelayMicroseconds(5000);
		if (++c == 6 * PWM_DEPTH) c = 0;
	}

}
